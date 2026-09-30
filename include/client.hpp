#ifndef C4_CLIENT_HPP
#define C4_CLIENT_HPP

#include "common.hpp"
#include "game.hpp"
#include "message.pb.h"
#include "ring_buffer.hpp"

namespace C4 {
typedef enum {
  CLIENT_STATE_LOBBY,
  CLIENT_STATE_IN_GAME,
} client_state_t;

class Client {
public:
  class MessageHandler {
  public:
    Client *m_client;

    MessageHandler(Client *client);
    ~MessageHandler();

    void handle_message(Message* msg);
    void handle_event(const EventPayload* event);
    void handle_event_server_connected(const EventServerConnected* sc);
    void handle_event_lobby_updated(const EventLobbyUpdated* lu);
  };

  uv_loop_t *m_loop;
  char *m_host;
  int m_port;
  uv_tcp_t m_client;
  struct sockaddr_in m_server_addr;
  RingBuffer m_ring_buf;
  client_id_t m_client_id;
  uint64_t m_total_clients;
  client_state_t m_state;
  Game m_game;
  MessageHandler m_msg_hdl;

  Client(uv_loop_t *loop = g_loop, const char *host = "localhost",
         int port = 8080);
  ~Client();

  void run();

  static void on_connect(uv_connect_t *connect, int status);
  static void on_read(uv_stream_t *stream, long nread, const uv_buf_t *buf);
  static void on_alloc(uv_handle_t *handle, unsigned long size, uv_buf_t *buf);
  static void on_write(uv_write_t *write, int status);
  static void on_close(uv_handle_t *handle);
};
}; // namespace C4

#endif