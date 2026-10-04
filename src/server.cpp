#include "server.hpp"
#include "common.hpp"
#include "game.hpp"

using namespace C4;

Server::Server(int port, int backlog) : m_msg_hdl(this) {
  m_loop = uv_default_loop();
  m_port = port;
  m_backlog = backlog;

  uv_tcp_init(m_loop, &m_server);
  m_server.data = this;

  m_session_cid = 0;
  m_game_cid = 0;
}

Server::~Server() {}

void Server::run() {
  uv_ip4_addr(NULL, m_port, &m_server_addr);
  uv_tcp_bind(&m_server, (struct sockaddr *)&m_server_addr, 0);
  uv_listen((uv_stream_t *)&m_server, m_backlog, on_connection);
  spdlog::info("Listening on port {}", m_port);
  uv_run(m_loop, UV_RUN_DEFAULT);
}

void Server::on_connection(uv_stream_t *stream, int status) {
  Server *self = (Server *)stream->data;
  if (status < 0) {
    spdlog::error("Client connection error");
    return;
  }
  Session *session = new Session(self->m_loop, self, ++self->m_session_cid);
  if (uv_accept((uv_stream_t *)&self->m_server,
                (uv_stream_t *)&session->m_session) != 0) {
    spdlog::error("Client accept error");
    delete session;
    return;
  }
  spdlog::info("New client connection with ID: {}", self->m_session_cid);
  self->m_sessions_map[self->m_session_cid] = session;
  self->m_msg_hdl.broadcast_event_lobby_updated();
  session->run();
}

void Server::disconnect(session_id_t session_id) {
  Session *session = m_sessions_map[session_id];
  if (session == NULL || m_sessions_map.erase(session_id) != 1)
    return;
  if (session->m_curr_game_id) {
    Game *game = m_game_map[session->m_curr_game_id];
    if (game == NULL) {
      return;
    }
    if (m_game_map.erase(game->m_id) != 1) {
      return;
    }
    delete game;
  }
  delete session;
  m_msg_hdl.broadcast_event_lobby_updated();
}

void Server::create_new_game(session_id_t session_id) {
  printf("Creating new game\n");
  Session *session = m_sessions_map[session_id];
  if (session == NULL || session->m_curr_game_id != 0)
    return;
  Game *game = new Game(++m_game_cid, session->m_id);
  m_game_map[game->m_id] = game;
  session->m_curr_game_id = game->m_id;
}

void Server::join_game(session_id_t session_id, game_id_t game_id) {
  Session *session = m_sessions_map[session_id];
  Game *game = m_game_map[game_id];
  if (session == NULL)
    return;
  if (game == NULL)
    return;
  game->m_p2_id = session_id;
  game->start();
}
