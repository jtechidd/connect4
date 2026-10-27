#include "server.hpp"
#include "common.hpp"
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
  uv_ip4_addr(NULL, m_port, &m_server_address);
  uv_tcp_bind(&m_uv_tcp_server, (struct sockaddr *)&m_server_address, 0);
  uv_listen((uv_stream_t *)&m_uv_tcp_server, m_backlog, on_uv_tcp_server_connection);
  uv_timer_start(&m_uv_timer_broadcast_event_lobby_updated, Server::on_uv_timer_broadcast_event_lobby_updated_timeout,
                 SERVER_BROADCAST_EVENT_LOBBY_UPDATED_INTERVAL, SERVER_BROADCAST_EVENT_LOBBY_UPDATED_INTERVAL);
  spdlog::info("Listening on port [{}].", m_port);
  uv_run(m_uv_loop, UV_RUN_DEFAULT);
}

void Server::on_uv_tcp_server_connection(uv_stream_t *stream, int status) {
  Server *self = (Server *)stream->data;
  if (status < 0) {
    spdlog::error("Client connection error.");
    return;
  }
  ServerClientConnection *connection = new ServerClientConnection(self->m_uv_loop, self, ++self->m_client_cid);
  if (uv_accept((uv_stream_t *)&self->m_uv_tcp_server, (uv_stream_t *)&connection->m_uv_tcp_connection) != 0) {
    spdlog::error("Client accept error.");
    delete connection;
    return;
  }
  spdlog::info("New connection associated to client with ID [{}].", self->m_client_cid);
  self->m_clients_map[self->m_client_cid] = connection;
  connection->run();
}

void Server::disconnect(client_id_t client_id) {
  spdlog::info("Disconnecting client with ID [{}].", client_id);
  ServerClientConnection *connection = find_client_by_id(client_id);
  if (connection == NULL || m_clients_map.erase(client_id) != 1) {
    spdlog::error("Cannot erase connection associated to client with ID [{}] from the mapping.", client_id);
    return;
  }
  if (connection->m_client_data.m_current_game_id) {
    Game *game = find_game_by_id(connection->m_client_data.m_current_game_id);
    if (game) {
      game->player_left(client_id);
      if (game->m_player1_left && game->m_player2_left)
        delete_game(game->m_id);
      else if (game->m_player1_left)
        m_message_handler.emit_event_game_updated(game->m_player2_id, game->m_id);
      else if (game->m_player2_left)
        m_message_handler.emit_event_game_updated(game->m_player1_id, game->m_id);
    }
  }
  delete connection;
  spdlog::debug("Successfully delete connection associated to client with ID [{}].", client_id);
}

void Server::create_new_game(client_id_t client1_id, client_id_t client2_id) {
  spdlog::info("Creating new game for both clients with ID [{}] and [{}].", client1_id, client2_id);
  ServerClientConnection *client1 = find_client_by_id(client1_id);
  if (!(client1 && client1->m_client_data.is_ready_for_new_game())) {
    spdlog::info("Client with ID [{}] is not ready for new game, skipping.", client1_id);
    return;
  }
  ServerClientConnection *client2 = find_client_by_id(client2_id);
  if (!(client2 && client2->m_client_data.is_ready_for_new_game())) {
    spdlog::info("Client with ID [{}] is not ready for new game, skipping.", client2_id);
    return;
  }
  Game *game = new Game(++m_game_cid, client1_id, client2_id);
  m_game_map[game->m_id] = game;

  client1->m_client_data.m_current_game_id = game->m_id;
  client1->m_client_data.m_state = CLIENT_STATE_IN_GAME;
  client2->m_client_data.m_current_game_id = game->m_id;
  client2->m_client_data.m_state = CLIENT_STATE_IN_GAME;
  m_message_handler.emit_event_client_data_updated(client1_id);
  m_message_handler.emit_event_client_data_updated(client2_id);

  game->start();
  m_message_handler.emit_event_game_updated(client1_id, game->m_id);
  m_message_handler.emit_event_game_updated(client2_id, game->m_id);
}

void Server::delete_game(game_id_t game_id) {
  spdlog::info("Deleting game with ID [{}].", game_id);
  Game *game = find_game_by_id(game_id);
  if (game == NULL) {
    spdlog::warn("Cannot find game with ID [{}].", game_id);
    return;
  }
  if (!(game->m_player1_left && game->m_player2_left)) {
    spdlog::warn("Both player are not quit.");
    return;
  }
  if (m_game_map.erase(game_id) != 1) {
    spdlog::warn("Cannot erase game from map.");
    return;
  }
  delete game;
  spdlog::debug("Successfully delete game with ID [{}]", game_id);
}

ServerClientConnection *Server::find_client_by_id(client_id_t client_id) {
  auto it = m_clients_map.find(client_id);
  if (it == m_clients_map.end())
    return NULL;
  assert(it->second != NULL);
  return it->second;
}

Game *Server::find_game_by_id(game_id_t game_id) {
  auto it = m_game_map.find(game_id);
  if (it == m_game_map.end())
    return NULL;
  assert(it->second != NULL);
  return it->second;
}

void Server::on_uv_timer_broadcast_event_lobby_updated_timeout(uv_timer_t *timer) {
  Server *self = (Server *)timer->data;
  self->m_message_handler.broadcast_event_lobby_updated();
}

}; // namespace C4