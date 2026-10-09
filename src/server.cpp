#include "server.hpp"
#include "common.hpp"
#include "game.hpp"

namespace C4 {

Server::Server(int port, int backlog) : m_message_handler(this) {
  m_uv_loop = g_uv_loop;
  m_port = port;
  m_backlog = backlog;
  m_client_cid = 0;
  m_game_cid = 0;
  uv_tcp_init(m_uv_loop, &m_uv_tcp_server);
  m_uv_tcp_server.data = this;
  uv_timer_init(m_uv_loop, &m_uv_timer_broadcast_event_lobby_updated);
  m_uv_timer_broadcast_event_lobby_updated.data = this;
}

Server::~Server() {}

void Server::run() {
  uv_ip4_addr(NULL, m_port, &m_server_addr);
  uv_tcp_bind(&m_uv_tcp_server, (struct sockaddr *)&m_server_addr, 0);
  uv_listen((uv_stream_t *)&m_uv_tcp_server, m_backlog, on_uv_tcp_server_connection);
  uv_timer_start(&m_uv_timer_broadcast_event_lobby_updated, Server::on_uv_timer_broadcast_event_lobby_updated_timeout, 10000, 10000);
  spdlog::info("Listening on port {}", m_port);
  uv_run(m_uv_loop, UV_RUN_DEFAULT);
}

void Server::on_uv_tcp_server_connection(uv_stream_t *stream, int status) {
  Server *self = (Server *)stream->data;
  if (status < 0) {
    spdlog::error("Client connection error");
    return;
  }
  ClientConnection *connection = new ClientConnection(self->m_uv_loop, self, ++self->m_client_cid);
  if (uv_accept((uv_stream_t *)&self->m_uv_tcp_server, (uv_stream_t *)&connection->m_uv_tcp_connection) != 0) {
    spdlog::error("Client accept error");
    delete connection;
    return;
  }
  spdlog::info("New client connection with ID: {}", self->m_client_cid);
  self->m_clients_map[self->m_client_cid] = connection;
  connection->run();
}

void Server::disconnect(client_id_t client_id) {
  ClientConnection *connection = m_clients_map[client_id];
  if (connection == NULL || m_clients_map.erase(client_id) != 1)
    return;
  if (connection->m_current_game_id) {
    Game *game = m_game_map[connection->m_current_game_id];
    if (game == NULL) {
      return;
    }
    if (m_game_map.erase(game->m_id) != 1) {
      return;
    }
    delete game;
  }
  delete connection;
}

void Server::create_new_game(client_id_t client_id) {
  ClientConnection *connection = m_clients_map[client_id];
  if (connection == NULL || connection->m_current_game_id != 0)
    return;
  Game *game = new Game(++m_game_cid, connection->m_id);
  m_game_map[game->m_id] = game;
  connection->m_current_game_id = game->m_id;
}

void Server::join_game(client_id_t client_id, game_id_t game_id) {
  ClientConnection *connection = m_clients_map[client_id];
  Game *game = m_game_map[game_id];
  if (connection == NULL)
    return;
  if (game == NULL)
    return;
  game->m_p2_id = client_id;
  game->start();
}

void Server::on_uv_timer_broadcast_event_lobby_updated_timeout(uv_timer_t *timer) {
  Server *self = (Server *)timer->data;
  self->m_message_handler.broadcast_event_lobby_updated();
}

}; // namespace C4