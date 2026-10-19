#ifndef C4_SERVER_CLIENT_CONNECTION_HPP
#define C4_SERVER_CLIENT_CONNECTION_HPP

#include "common.hpp"
#include "ring_buffer.hpp"

namespace C4 {
class ServerClientConnection {
public:
  uv_loop_t *m_uv_loop;
  Server *m_server;
  client_id_t m_id;
  uv_tcp_t m_uv_tcp_connection;
  char m_username[USERNAME_MAX_SIZE + 1];
  ClientState m_state;
  RingBuffer m_ring_buffer;
  game_id_t m_current_game_id;

  ServerClientConnection(uv_loop_t *loop, Server *server, client_id_t id);
  ~ServerClientConnection();

  void run();

  void emit_event_server_connected();

  // UV callbacks
  static void on_uv_tcp_connection_alloc(uv_handle_t *handle, unsigned long size, uv_buf_t *buf);
  static void on_uv_tcp_connection_read(uv_stream_t *stream, long nread, const uv_buf_t *buf);
  static void on_uv_tcp_connection_close(uv_handle_t *handle);
  static void on_uv_tcp_connection_write(uv_write_t *write, int status);
};
} // namespace C4

#endif