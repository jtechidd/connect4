#include "common.hpp"
#include "server.hpp"
#include "game.hpp"

using namespace C4;

Server::MessageHandler::MessageHandler(Server *server) { m_server = server; }
Server::MessageHandler::~MessageHandler() {}

void Server::MessageHandler::send_message(session_id_t session_id,
                                          Message *msg) {
  Session *s = m_server->m_sessions_map[session_id];
  if (!s)
    return;
  WriteRequest *wr = new WriteRequest(msg);
  wr->req.data = this;
  uv_write((uv_write_t *)wr, (uv_stream_t *)&s->m_session, &wr->buf, 1,
           Server::Session::on_write);
}

void Server::MessageHandler::broadcast_message(Message *msg) {
  for (auto [id, _] : m_server->m_sessions_map) {
    send_message(id, msg);
  }
}

void Server::MessageHandler::emit_event_server_connected(
    session_id_t session_id) {
  Message msg;
  EventPayload *ep = msg.mutable_event_payload();
  EventServerConnected *sc = ep->mutable_event_server_connected();
  sc->set_client_id(session_id);
  send_message(session_id, &msg);
}

void Server::MessageHandler::broadcast_event_lobby_updated() {
  Message msg;
  EventPayload *ep = msg.mutable_event_payload();
  EventLobbyUpdated *lu = ep->mutable_event_lobby_updated();
  lu->set_total_clients(m_server->m_sessions_map.size());
  lu->set_total_games(m_server->m_game_map.size());
  for (auto [_, g] : m_server->m_game_map) {
    GameStatus *gs = lu->add_games();
    gs->set_id(g->m_id);
    gs->set_available(!g->m_p1_id || !g->m_p2_id);
    gs->set_player1_id(g->m_p1_id);
    gs->set_player2_id(g->m_p2_id);
  }
  broadcast_message(&msg);
}

void Server::MessageHandler::handle_message(Message *msg) {
  // TODO: implement
}