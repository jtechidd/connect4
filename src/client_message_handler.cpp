#include "client_message_handler.hpp"
#include "client.hpp"
#include "game.hpp"
#include "message.pb.h"

namespace C4 {

ClientMessageHandler::ClientMessageHandler(Client *client) { m_client = client; }
ClientMessageHandler::~ClientMessageHandler() {}

void ClientMessageHandler::handle_message(Message *message) {
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

void ClientMessageHandler::handle_event(const EventPayload *event_payload) {
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
  case EventPayload::kEventGameUpdated:
    handle_event_game_updated(&event_payload->event_game_updated());
  default:
    break;
  }
}

void ClientMessageHandler::handle_event_server_connected(const EventServerConnected *payload) {
  spdlog::info("Handling event [server_connected].");
  m_client->m_data.m_client_id = payload->client_id();
  spdlog::debug("Set client ID to [{}].", m_client->m_data.m_client_id);
}

void ClientMessageHandler::handle_event_lobby_entered(const EventLobbyEntered *payload) {
  spdlog::info("Handling event [lobby_entered].");
  m_client->m_data.m_state = CLIENT_STATE_LOBBY;
  m_client->m_ui.lobby_entered();
}

void ClientMessageHandler::handle_event_username_check_failed(const EventUsernameCheckFailed *payload) {
  spdlog::info("Handling event [username_check_failed].");
  m_client->m_ui.username_check_failed();
}

void ClientMessageHandler::handle_event_lobby_updated(const EventLobbyUpdated *payload) {
  spdlog::info("Handling event [lobby_updated].");
  m_client->m_total_clients = payload->total_clients();
  m_client->m_ui.lobby_updated(payload);
  spdlog::debug("Set total clients to [{}].", m_client->m_total_clients);
}

void ClientMessageHandler::handle_event_new_game_invite_received(const EventNewGameInviteReceived *payload) {
  spdlog::info("Handling event [new_game_invite_received].");
  for (NewGameInvite invite : m_client->m_new_game_invite_queue) {
    if (invite.inviter_client_id == payload->inviter_client_id()) {
      spdlog::debug("Inviter client ID [{}] is already in queue, skipping.", invite.inviter_client_id);
      return;
    }
  }
  NewGameInvite invite;
  invite.inviter_client_id = payload->inviter_client_id();
  snprintf(invite.inviter_username, sizeof(invite.inviter_username), "%s", payload->inviter_username().c_str());
  m_client->m_new_game_invite_queue.push_back(invite);
  m_client->m_ui.update_invite();
  spdlog::debug("Queued invite from client ID [{}].", invite.inviter_client_id);
}

void ClientMessageHandler::handle_event_new_game_joined(const EventNewGameJoined *payload) {
  spdlog::info("Handling event [new_game_joined]");
  m_client->m_game.m_id = payload->game_id();
  m_client->m_data.m_current_game_id = payload->game_id();
  m_client->m_game.m_player1_id = payload->player1_id();
  m_client->m_game.m_player2_id = payload->player2_id();
  m_client->m_game.m_opp_id = payload->opponent_client_id();
  snprintf(m_client->m_game.m_opp_username, sizeof(m_client->m_game.m_opp_username), "%s", payload->opponent_username().c_str());
  m_client->m_ui.new_game_joined();
  m_client->m_ui.update_game();
}

void ClientMessageHandler::handle_event_game_updated(const EventGameUpdated *payload) {
  spdlog::info("Handling event [game_updated]");
  if (!(payload->game_id() == m_client->m_game.m_id && payload->game_id() == m_client->m_data.m_current_game_id)) {
    spdlog::warn("Payload game ID [{}] mismatch from game ID [{}] or player current game ID [{}]", payload->game_id(), m_client->m_game.m_id,
                 m_client->m_data.m_current_game_id);
    return;
  }
  m_client->m_game.m_round = payload->round();
  m_client->m_game.m_state = payload->state();
  m_client->m_game.m_turn = payload->turn();
  m_client->m_game.m_verdict = payload->verdict();
  for (int i = 0; i < payload->board_size(); i++) {
    m_client->m_game.m_board[i / 7][i % 7] = (GamePlayer)payload->board().at(i);
  }
  for (int i = 0; i < payload->connect_mask_size(); i++) {
    m_client->m_game.m_connect_mask[i / 7][i % 7] = payload->connect_mask().at(i);
  }
  m_client->m_ui.update_game();
}

void ClientMessageHandler::send_message(Message *msg) {
  WriteRequest *write_req = new WriteRequest(msg);
  write_req->req.data = this;
  spdlog::debug("Sending message with [{}] bytes.", write_req->buf.len);
  uv_write((uv_write_t *)write_req, (uv_stream_t *)&m_client->m_uv_tcp_connection, &write_req->buf, 1, Client::on_uv_tcp_connection_write);
}

void ClientMessageHandler::send_command_enter_lobby() {
  Message msg;
  CommandEnterLobby *payload = msg.mutable_command_payload()->mutable_command_enter_lobby();
  payload->set_username(m_client->m_data.m_username);
  spdlog::info("Sending command [enter_lobby] with username [{}].", m_client->m_data.m_username);
  send_message(&msg);
}

void ClientMessageHandler::send_command_new_game_invite() {
  if (!m_client->m_selected_client_id)
    return;
  Message msg;
  CommandNewGameInvite *payload = msg.mutable_command_payload()->mutable_command_new_game_invite();
  payload->set_client_id(m_client->m_selected_client_id);
  spdlog::info("Sending command [new_game_invite] with selected client ID [{}].", m_client->m_selected_client_id);
  send_message(&msg);
}

void ClientMessageHandler::send_command_invite_accept() {
  if (m_client->m_new_game_invite_queue.size() == 0)
    return;
  Message msg;
  CommandInviteAccept *payload = msg.mutable_command_payload()->mutable_command_invite_accept();
  payload->set_inviter_client_id(m_client->m_new_game_invite_queue.front().inviter_client_id);
  m_client->m_new_game_invite_queue.pop_front();
  m_client->m_ui.update_invite();
  spdlog::info("Sending command [invite_accept] with inviter client ID [{}].", payload->inviter_client_id());
  send_message(&msg);
}

void ClientMessageHandler::send_command_player_make_move(int col) {
  Message msg;
  CommandPlayerMakeMove *payload = msg.mutable_command_payload()->mutable_command_player_make_move();
  payload->set_game_id(m_client->m_game.m_id);
  payload->set_player_id(m_client->m_data.m_client_id);
  payload->set_played_column(col);
  spdlog::info("Sending command [player_make_move] with played column [{}]", col);
  send_message(&msg);
}

}; // namespace C4