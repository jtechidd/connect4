#include "client.hpp"
#include "message.pb.h"

namespace C4 {

Client::MessageHandler::MessageHandler(Client *client) { m_client = client; }
Client::MessageHandler::~MessageHandler() {}

void Client::MessageHandler::handle_message(Message *message) {
  switch (message->payload_type_case()) {
  case Message::kEventPayload:
    handle_event(&message->event_payload());
    break;
  // Client will only receive events
  case Message::PAYLOAD_TYPE_NOT_SET:
  default:
    break;
  }
}

void Client::MessageHandler::handle_event(const EventPayload *event_payload) {
  switch (event_payload->payload_case()) {
  case EventPayload::kEventServerConnected:
    handle_event_server_connected(&event_payload->event_server_connected());
    break;
  case EventPayload::kEventLobbyEntered:
    handle_event_lobby_entered(&event_payload->event_lobby_entered());
    break;
  case EventPayload::kEventUsernameCheckFailed:
    handle_event_username_check_failed(&event_payload->event_username_check_failed());
    break;
  case EventPayload::kEventLobbyUpdated:
    handle_event_lobby_updated(&event_payload->event_lobby_updated());
    break;
  case EventPayload::kEventNewGameInviteReceived:
    handle_event_new_game_invite_received(&event_payload->event_new_game_invite_received());
    break;
  case EventPayload::kEventNewGameJoined:
    handle_event_new_game_joined(&event_payload->event_new_game_joined());
    break;
  default:
    break;
  }
}

void Client::MessageHandler::handle_event_server_connected(const EventServerConnected *payload) {
  m_client->m_client_id = payload->client_id();
  spdlog::info("Set client ID to {}", m_client->m_client_id);
}

void Client::MessageHandler::handle_event_lobby_entered(const EventLobbyEntered *payload) {
  m_client->m_state = CLIENT_STATE_LOBBY;
  m_client->m_qt_ui.lobby_entered();
  spdlog::info("Entered lobby");
}

void Client::MessageHandler::handle_event_username_check_failed(const EventUsernameCheckFailed *payload) {
  m_client->m_qt_ui.username_check_failed();
  spdlog::info("Username check failed");
}

void Client::MessageHandler::handle_event_lobby_updated(const EventLobbyUpdated *payload) {
  m_client->m_total_clients = payload->total_clients();
  m_client->m_qt_ui.lobby_updated(payload);
  spdlog::info("Set total clients to {}", m_client->m_total_clients);
}

void Client::MessageHandler::handle_event_new_game_invite_received(const EventNewGameInviteReceived *payload) {
  for (NewGameInvite invite : m_client->m_new_game_invite_queue)
    if (invite.inviter_client_id == payload->inviter_client_id())
      return;
  NewGameInvite invite;
  invite.inviter_client_id = payload->inviter_client_id();
  snprintf(invite.inviter_username, sizeof(invite.inviter_username), "%s", payload->inviter_username().c_str());
  m_client->m_new_game_invite_queue.push_back(invite);
  m_client->m_qt_ui.update_invite();
}

void Client::MessageHandler::handle_event_new_game_joined(const EventNewGameJoined *payload) {
  // TODO: in game ui
}

void Client::MessageHandler::send_message(Message *msg) {
  WriteRequest *write_req = new WriteRequest(msg);
  write_req->req.data = this;
  spdlog::debug("Sending message with {} bytes", write_req->buf.len);
  uv_write((uv_write_t *)write_req, (uv_stream_t *)&m_client->m_uv_tcp_connection, &write_req->buf, 1, Client::on_uv_tcp_connection_write);
}

void Client::MessageHandler::send_command_enter_lobby() {
  Message msg;
  CommandEnterLobby *payload = msg.mutable_command_payload()->mutable_command_enter_lobby();
  payload->set_username(m_client->m_username);
  spdlog::info("Entering lobby with username: {}", m_client->m_username);
  send_message(&msg);
}

void Client::MessageHandler::send_command_new_game_invite() {
  if (!m_client->m_selected_client_id)
    return;
  Message msg;
  CommandNewGameInvite *payload = msg.mutable_command_payload()->mutable_command_new_game_invite();
  payload->set_client_id(m_client->m_selected_client_id);
  spdlog::info("Inviting client: {}", m_client->m_selected_client_id);
  send_message(&msg);
}

void Client::MessageHandler::send_command_invite_accept() {
  if (m_client->m_new_game_invite_queue.size() == 0)
    return;
  Message msg;
  CommandInviteAccept *payload = msg.mutable_command_payload()->mutable_command_invite_accept();
  payload->set_inviter_client_id(m_client->m_new_game_invite_queue.front().inviter_client_id);
  m_client->m_new_game_invite_queue.pop_front();
  m_client->m_qt_ui.update_invite();
  spdlog::info("Accepting invite from client: {}", payload->inviter_client_id());
  send_message(&msg);
}

}; // namespace C4