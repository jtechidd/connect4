#include "server.hpp"
#include "game.hpp"
#include "server_client_connection.hpp"

namespace C4 {

Server::Server(int port, int backlog) : m_message_handler(this) {
  m_uv_loop = uv_default_loop();
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
  uv_timer_start(&m_uv_timer_broadcast_event_lobby_updated, Server::on_uv_timer_broadcast_event_lobby_updated_timeout,
                 SERVER_BROADCAST_EVENT_LOBBY_UPDATED_INTERVAL, SERVER_BROADCAST_EVENT_LOBBY_UPDATED_INTERVAL);
  spdlog::info("Listening on port {}", m_port);
  uv_run(m_uv_loop, UV_RUN_DEFAULT);
}

void Server::on_uv_tcp_server_connection(uv_stream_t *stream, int status) {
  Server *self = (Server *)stream->data;
  if (status < 0) {
    spdlog::error("Client connection error");
    return;
  }
  ServerClientConnection *connection = new ServerClientConnection(self->m_uv_loop, self, ++self->m_client_cid);
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
  spdlog::info("Disconnecting client ID: {}", client_id);
  ServerClientConnection *connection = m_clients_map[client_id];
  if (connection == NULL || m_clients_map.erase(client_id) != 1) {
    return;
  }
  if (connection->m_client_data.m_current_game_id) {
    Game *game = m_game_map[connection->m_client_data.m_current_game_id];
    if (game) {
      if (game->m_p1_id == client_id)
        game->m_p1_id = 0;
      else if (game->m_p2_id == client_id)
        game->m_p2_id = 0;
    }
  }
  delete connection;
}

void Server::create_new_game(client_id_t client1_id, client_id_t client2_id) {
  ServerClientConnection *client1 = m_clients_map[client1_id];
  if (!(client1 && client1->m_client_data.is_ready_for_new_game())) {
    return;
  }
  ServerClientConnection *client2 = m_clients_map[client2_id];
  if (!(client2 && client2->m_client_data.is_ready_for_new_game())) {
    return;
  }
  Game *game = new Game(++m_game_cid, client1_id, client2_id);
  m_game_map[game->m_id] = game;
  client1->m_client_data.m_current_game_id = game->m_id;
  client2->m_client_data.m_current_game_id = game->m_id;
  m_message_handler.emit_event_new_game_joined(client1_id, game->m_id, client2_id);
  m_message_handler.emit_event_new_game_joined(client2_id, game->m_id, client1_id);
}

void Server::join_game(client_id_t client_id, game_id_t game_id) {
  ServerClientConnection *connection = m_clients_map[client_id];
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