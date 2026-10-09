#include "common.hpp"
#include "game.hpp"
#include "message.pb.h"
#include "server.hpp"

namespace C4 {

Server::MessageHandler::MessageHandler(Server *server) { m_server = server; }
Server::MessageHandler::~MessageHandler() {}

void Server::MessageHandler::send_message(client_id_t client_id, Message *msg) {
  ClientConnection *client_conn = m_server->m_clients_map[client_id];
  if (!client_conn)
    return;
  WriteRequest *write_req = new WriteRequest(msg);
  write_req->req.data = this;
  uv_write((uv_write_t *)write_req, (uv_stream_t *)&client_conn->m_uv_tcp_connection, &write_req->buf, 1, Server::ClientConnection::on_uv_tcp_connection_write);
}

void Server::MessageHandler::broadcast_message(Message *msg) {
  for (auto [id, _] : m_server->m_clients_map) {
    send_message(id, msg);
  }
}

void Server::MessageHandler::emit_event_server_connected(client_id_t client_id) {
  Message msg;
  EventServerConnected *payload = msg.mutable_event_payload()->mutable_event_server_connected();
  payload->set_client_id(client_id);
  send_message(client_id, &msg);
}

void Server::MessageHandler::init_event_lobby_updated(Message *msg) {
  EventLobbyUpdated *payload = msg->mutable_event_payload()->mutable_event_lobby_updated();
  payload->set_total_clients(m_server->m_clients_map.size());
  payload->set_total_games(m_server->m_game_map.size());
  for (auto [_, game] : m_server->m_game_map) {
    GameInfo *game_info = payload->add_games();
    game_info->set_id(game->m_id);
    game_info->set_player1_id(game->m_p1_id);
    game_info->set_player2_id(game->m_p2_id);
    game_info->set_state(game->m_state);
  }
  for (auto [_, c] : m_server->m_clients_map) {
    ClientInfo *client_info = payload->add_clients();
    client_info->set_id(c->m_id);
    client_info->set_state(c->m_state);
    client_info->set_username(c->m_username);
  }
}

void Server::MessageHandler::broadcast_event_lobby_updated() {
  spdlog::debug("Broadcasting event lobby updated to all clients...");
  Message msg;
  init_event_lobby_updated(&msg);
  broadcast_message(&msg);
}

void Server::MessageHandler::emit_event_lobby_updated(client_id_t client_id) {
  Message msg;
  init_event_lobby_updated(&msg);
  send_message(client_id, &msg);
}

void Server::MessageHandler::emit_event_lobby_entered(client_id_t client_id) {
  Message msg;
  msg.mutable_event_payload()->mutable_event_lobby_entered();
  send_message(client_id, &msg);
}

void Server::MessageHandler::emit_event_username_check_failed(client_id_t client_id) {
  Message msg;
  msg.mutable_event_payload()->mutable_event_username_check_failed();
  send_message(client_id, &msg);
}

void Server::MessageHandler::emit_event_new_game_invite_received(client_id_t client_id, client_id_t inviter_client_id) {
  ClientConnection *inviter_client = m_server->m_clients_map[inviter_client_id];
  if (!inviter_client)
    return;
  Message msg;
  EventNewGameInviteReceived *payload = msg.mutable_event_payload()->mutable_event_new_game_invite_received();
  payload->set_inviter_client_id(inviter_client_id);
  payload->set_inviter_username(inviter_client->m_username);
  send_message(client_id, &msg);
}

void Server::MessageHandler::emit_event_new_game_joined(client_id_t client_id, game_id_t game_id, client_id_t opp_client_id) {
  if (!m_server->m_clients_map[opp_client_id])
    return;
  Message msg;
  EventNewGameJoined *payload = msg.mutable_event_payload()->mutable_event_new_game_joined();
  payload->set_game_id(game_id);
  payload->set_opponent_client_id(opp_client_id);
  payload->set_opponent_username(m_server->m_clients_map[opp_client_id]->m_username);
  send_message(client_id, &msg);
}

void Server::MessageHandler::handle_message(client_id_t client_id, Message *msg) {
  switch (msg->payload_type_case()) {
  case Message::kEventPayload:
    handle_event(client_id, &msg->event_payload());
    break;
  case Message::kCommandPayload:
    handle_command(client_id, &msg->command_payload());
  case Message::PAYLOAD_TYPE_NOT_SET:
  default:
    break;
  }
}

void Server::MessageHandler::handle_event(client_id_t client_id, const EventPayload *event_payload) {
  switch (event_payload->payload_case()) {
  default:
    break;
  }
}

void Server::MessageHandler::handle_command(client_id_t client_id, const CommandPayload *command_payload) {
  switch (command_payload->payload_case()) {
  case CommandPayload::kCommandEnterLobby:
    handle_command_enter_lobby(client_id, &command_payload->command_enter_lobby());
    break;
  case CommandPayload::kCommandNewGameInvite:
    handle_command_new_game_invite(client_id, &command_payload->command_new_game_invite());
    break;
  case CommandPayload::kCommandInviteAccept:
    handle_command_invite_accept(client_id, &command_payload->command_invite_accept());
    break;
  default:
    break;
  }
}

void Server::MessageHandler::handle_command_enter_lobby(client_id_t client_id, const CommandEnterLobby *payload) {
  for (auto [_, client_conn] : m_server->m_clients_map) {
    if (!strcmp(client_conn->m_username, payload->username().c_str())) {
      emit_event_username_check_failed(client_id);
      return;
    }
  }
  ClientConnection *client_conn = m_server->m_clients_map[client_id];
  assert(client_conn != NULL);
  assert(payload->username().length() <= USERNAME_MAX_SIZE);
  snprintf(client_conn->m_username, sizeof(client_conn->m_username), "%s", payload->username().c_str());
  assert(m_server->m_clients_map[client_id] != nullptr);
  m_server->m_clients_map[client_id]->m_state = CLIENT_STATE_LOBBY;
  emit_event_lobby_entered(client_id);
  emit_event_lobby_updated(client_id);
}

void Server::MessageHandler::handle_command_new_game_invite(client_id_t client_id, const CommandNewGameInvite *payload) {
  emit_event_new_game_invite_received(payload->client_id(), client_id);
}

void Server::MessageHandler::handle_command_invite_accept(client_id_t client_id, const CommandInviteAccept *payload) {
  client_id_t client_id_1 = payload->inviter_client_id();
  client_id_t client_id_2 = client_id;
  // TODO: check availability, create new game
  game_id_t game_id = 1; // fake
  emit_event_new_game_joined(client_id_1, game_id, client_id_2);
  emit_event_new_game_joined(client_id_2, game_id, client_id_1);
}

}; // namespace C4