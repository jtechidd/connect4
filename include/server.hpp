#ifndef C4_SERVER_HPP
#define C4_SERVER_HPP

#include "common.hpp"
#include "server_messsage_handler.hpp"

namespace C4 {

constexpr int SERVER_DEFAULT_PORT = 8080;
constexpr int SERVER_DEFAULT_BACKLOG = 128;
constexpr uint64_t SERVER_BROADCAST_EVENT_LOBBY_UPDATED_INTERVAL = 5000;

class Server {
public:
  uv_loop_t *m_uv_loop;
  int m_port;
  int m_backlog;
  uv_tcp_t m_uv_tcp_server;
  struct sockaddr_in m_server_addr;
  client_id_t m_client_cid;
  std::map<client_id_t, ServerClientConnection *> m_clients_map;
  game_id_t m_game_cid;
  std::map<game_id_t, Game *> m_game_map;
  ServerMessageHandler m_message_handler;
  uv_timer_t m_uv_timer_broadcast_event_lobby_updated;

  Server(int port = SERVER_DEFAULT_PORT, int backlog = SERVER_DEFAULT_BACKLOG);
  ~Server();

  void run();

  void disconnect(client_id_t client_id);
  void create_new_game(client_id_t client1_id, client_id_t client2_id);
  void join_game(client_id_t client_id, game_id_t game_id);

  // UV callbacks
  static void on_uv_tcp_server_connection(uv_stream_t *stream, int status);
  static void on_uv_timer_broadcast_event_lobby_updated_timeout(uv_timer_t *timer);
};
} // namespace C4

#endif