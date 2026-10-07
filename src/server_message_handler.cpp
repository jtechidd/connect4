#include "common.hpp"
#include "game.hpp"
#include "message.pb.h"
#include "server.hpp"

namespace C4 {

Server::MessageHandler::MessageHandler(Server *server) { m_server = server; }
Server::MessageHandler::~MessageHandler() {}

void Server::MessageHandler::send_message(client_id_t client_id, Message *msg) {
  ClientConnection *s = m_server->m_clients_map[client_id];
  if (!s)
    return;
  WriteRequest *wr = new WriteRequest(msg);
  wr->req.data = this;
  uv_write((uv_write_t *)wr, (uv_stream_t *)&s->m_uv_tcp_connection, &wr->buf,
           1, Server::ClientConnection::on_uv_tcp_connection_write);
}

void Server::MessageHandler::broadcast_message(Message *msg) {
  for (auto [id, _] : m_server->m_clients_map) {
    send_message(id, msg);
  }
}

void Server::MessageHandler::emit_event_server_connected(
    client_id_t client_id) {
  Message msg;
  EventPayload *ep = msg.mutable_event_payload();
  EventServerConnected *sc = ep->mutable_event_server_connected();
  sc->set_client_id(client_id);
  send_message(client_id, &msg);
}

void Server::MessageHandler::init_event_lobby_updated(Message *msg) {
  EventPayload *ep = msg->mutable_event_payload();
  EventLobbyUpdated *lu = ep->mutable_event_lobby_updated();
  lu->set_total_clients(m_server->m_clients_map.size());
  lu->set_total_games(m_server->m_game_map.size());
  for (auto [_, g] : m_server->m_game_map) {
    GameInfo *gi = lu->add_games();
    gi->set_id(g->m_id);
    gi->set_player1_id(g->m_p1_id);
    gi->set_player2_id(g->m_p2_id);
    gi->set_state(g->m_state);
  }
  for (auto [_, c] : m_server->m_clients_map) {
    ClientInfo *ci = lu->add_clients();
    ci->set_id(c->m_id);
    ci->set_state(c->m_state);
    ci->set_username(c->m_username);
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
  EventPayload *ep = msg.mutable_event_payload();
  EventLobbyEntered *el = ep->mutable_event_lobby_entered();
  send_message(client_id, &msg);
}

void Server::MessageHandler::emit_event_username_check_failed(
    client_id_t client_id) {
  Message msg;
  EventPayload *ep = msg.mutable_event_payload();
  EventUsernameCheckFailed *ucf = ep->mutable_event_username_check_failed();
  send_message(client_id, &msg);
}

void Server::MessageHandler::handle_message(client_id_t client_id,
                                            Message *msg) {
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

void Server::MessageHandler::handle_event(client_id_t client_id,
                                          const EventPayload *event_payload) {
  switch (event_payload->payload_case()) {
  default:
    break;
  }
}

void Server::MessageHandler::handle_command(
    client_id_t client_id, const CommandPayload *command_payload) {
  switch (command_payload->payload_case()) {
  case CommandPayload::kCommandEnterLobby:
    handle_command_enter_lobby(client_id,
                               &command_payload->command_enter_lobby());
    break;
  default:
    break;
  }
}

void Server::MessageHandler::handle_command_enter_lobby(
    client_id_t client_id, const CommandEnterLobby *el) {
  for (auto [_, s] : m_server->m_clients_map) {
    if (!strcmp(s->m_username, el->username().c_str())) {
      emit_event_username_check_failed(client_id);
      return;
    }
  }
  auto s = m_server->m_clients_map[client_id];
  assert(s != NULL);
  assert(el->username().length() <= USERNAME_MAX_SIZE);
  strncpy(s->m_username, el->username().c_str(), el->username().length());
  assert(m_server->m_clients_map[client_id] != nullptr);
  m_server->m_clients_map[client_id]->m_state = CLIENT_STATE_LOBBY;
  emit_event_lobby_entered(client_id);
  emit_event_lobby_updated(client_id);
}

}; // namespace C4