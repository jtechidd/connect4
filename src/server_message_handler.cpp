#include "game.hpp"
#include "server.hpp"
#include "server_client_connection.hpp"

namespace C4 {

ServerMessageHandler::ServerMessageHandler(Server *server) { m_server = server; }
ServerMessageHandler::~ServerMessageHandler() {}

void ServerMessageHandler::send_message(client_id_t client_id, Message *msg) {
  ServerClientConnection *client_conn = m_server->m_clients_map[client_id];
  if (!client_conn)
    return;
  WriteRequest *write_req = new WriteRequest(msg);
  write_req->req.data = this;
  uv_write((uv_write_t *)write_req, (uv_stream_t *)&client_conn->m_uv_tcp_connection, &write_req->buf, 1, ServerClientConnection::on_uv_tcp_connection_write);
}

void ServerMessageHandler::broadcast_message(Message *msg) {
  for (auto [id, _] : m_server->m_clients_map) {
    send_message(id, msg);
  }
}

void ServerMessageHandler::emit_event_server_connected(client_id_t client_id) {
  Message msg;
  EventServerConnected *payload = msg.mutable_event_payload()->mutable_event_server_connected();
  payload->set_client_id(client_id);
  send_message(client_id, &msg);
}

void ServerMessageHandler::init_event_lobby_updated(Message *msg) {
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
  for (auto [_, conn] : m_server->m_clients_map) {
    ClientInfo *client_info = payload->add_clients();
    client_info->set_id(conn->m_client_data.m_client_id);
    client_info->set_state(conn->m_client_data.m_state);
    client_info->set_username(conn->m_client_data.m_username);
  }
}

void ServerMessageHandler::broadcast_event_lobby_updated() {
  spdlog::debug("Broadcasting event [lobby_updated] to all clients.");
  Message msg;
  init_event_lobby_updated(&msg);
  broadcast_message(&msg);
}

void ServerMessageHandler::emit_event_lobby_updated(client_id_t client_id) {
  spdlog::debug("Emitting event [lobby_updated] to client with ID [{}].", client_id);
  Message msg;
  init_event_lobby_updated(&msg);
  send_message(client_id, &msg);
}

void ServerMessageHandler::emit_event_lobby_entered(client_id_t client_id) {
  spdlog::debug("Emitting event [lobby_entered] to client with ID [{}].", client_id);
  Message msg;
  msg.mutable_event_payload()->mutable_event_lobby_entered();
  send_message(client_id, &msg);
}

void ServerMessageHandler::emit_event_username_check_failed(client_id_t client_id) {
  spdlog::debug("Emitting event [username_check_failed] to client with ID [{}].", client_id);
  Message msg;
  msg.mutable_event_payload()->mutable_event_username_check_failed();
  send_message(client_id, &msg);
}

void ServerMessageHandler::emit_event_new_game_invite_received(client_id_t client_id, client_id_t inviter_client_id) {
  spdlog::debug("Emitting event [new_game_invite_received] to client with ID [{}].", client_id);
  ServerClientConnection *inviter_client = m_server->m_clients_map[inviter_client_id];
  if (!inviter_client) {
    spdlog::debug("Inviter client with ID [{}] not found, skipping.", inviter_client_id);
    return;
  }
  Message msg;
  EventNewGameInviteReceived *payload = msg.mutable_event_payload()->mutable_event_new_game_invite_received();
  payload->set_inviter_client_id(inviter_client_id);
  payload->set_inviter_username(inviter_client->m_client_data.m_username);
  send_message(client_id, &msg);
}

void ServerMessageHandler::emit_event_new_game_joined(client_id_t client_id, game_id_t game_id, client_id_t opp_client_id) {
  spdlog::debug("Emitting event [new_game_joined] to client with ID [{}].", client_id);
  if (!m_server->m_clients_map[opp_client_id]) {
    spdlog::debug("Opponent client with ID [{}] not found, skipping.", opp_client_id);
    return;
  }
  Message msg;
  EventNewGameJoined *payload = msg.mutable_event_payload()->mutable_event_new_game_joined();
  payload->set_game_id(game_id);
  payload->set_opponent_client_id(opp_client_id);
  payload->set_opponent_username(m_server->m_clients_map[opp_client_id]->m_client_data.m_username);
  send_message(client_id, &msg);
}

void ServerMessageHandler::handle_message(client_id_t client_id, Message *msg) {
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

void ServerMessageHandler::handle_event(client_id_t client_id, const EventPayload *event_payload) {
  switch (event_payload->payload_case()) {
  default:
    break;
  }
}

void ServerMessageHandler::handle_command(client_id_t client_id, const CommandPayload *command_payload) {
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

void ServerMessageHandler::handle_command_enter_lobby(client_id_t client_id, const CommandEnterLobby *payload) {
  spdlog::info("Handling command [enter_lobby] from client with ID [{}].", client_id);
  for (auto [_, client_conn] : m_server->m_clients_map) {
    if (!strcmp(client_conn->m_client_data.m_username, payload->username().c_str())) {
      spdlog::info("Username [{}] is already taken, rejecting.", client_conn->m_client_data.m_username);
      emit_event_username_check_failed(client_id);
      return;
    }
  }
  ServerClientConnection *client_conn = m_server->m_clients_map[client_id];
  assert(client_conn != NULL);
  assert(payload->username().length() <= USERNAME_MAX_SIZE);
  snprintf(client_conn->m_client_data.m_username, sizeof(client_conn->m_client_data.m_username), "%s", payload->username().c_str());
  assert(m_server->m_clients_map[client_id] != nullptr);
  m_server->m_clients_map[client_id]->m_client_data.m_state = CLIENT_STATE_LOBBY;
  emit_event_lobby_entered(client_id);
  emit_event_lobby_updated(client_id);
}

void ServerMessageHandler::handle_command_new_game_invite(client_id_t client_id, const CommandNewGameInvite *payload) {
  spdlog::info("Handling command [new_game_invite] from client with ID [{}].", client_id);
  emit_event_new_game_invite_received(payload->client_id(), client_id);
}

void ServerMessageHandler::handle_command_invite_accept(client_id_t client_id, const CommandInviteAccept *payload) {
  spdlog::info("Handling command [invite_accept] from client with ID [{}]", client_id);
  client_id_t client1_id = payload->inviter_client_id();
  client_id_t client2_id = client_id;
  m_server->create_new_game(client1_id, client2_id);
}

}; // namespace C4