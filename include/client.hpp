#ifndef C4_CLIENT_HPP
#define C4_CLIENT_HPP

#include "client_data.hpp"
#include "client_message_handler.hpp"
#include "client_ui.hpp"
#include "common.hpp"
#include "game.hpp"
#include "message.pb.h"
#include "ring_buffer.hpp"
#include <QtWidgets>

namespace C4 {

constexpr const char *CLIENT_DEFAULT_HOST = "localhost";
constexpr int CLIENT_DEFAULT_PORT = 8080;
constexpr uint64_t CLIENT_TRY_CONNECT_INTERVAL = 1000;

class NewGameInvite {
public:
  client_id_t inviter_client_id;
  char inviter_username[USERNAME_MAX_SIZE + 1];
};

class Client {
public:
  char *m_host;
  int m_port;

  uv_loop_t *m_uv_loop;
  std::thread m_thread_uv_loop;

  uv_timer_t m_uv_timer_try_connect;
  uv_tcp_t m_uv_tcp_connection;
  struct sockaddr_in m_server_address;
  RingBuffer m_ring_buffer;
  ClientMessageHandler m_message_handler;

  char m_username_input[USERNAME_MAX_SIZE + 1];
  bool m_is_connected;
  client_id_t m_selected_client_id;
  uint64_t m_total_clients;
  std::deque<NewGameInvite> m_new_game_invite_queue;

  // Shared
  ClientData m_data;
  Game m_game;

  QApplication m_qapp;
  ClientUI m_ui;

  // UV async handles
  uv_async_t m_uv_async_keep_alive;
  // From main thread (UI)
  uv_async_t m_uv_async_stop_loop;
  uv_async_t m_uv_async_enter_lobby;
  uv_async_t m_uv_async_new_game_invite;
  uv_async_t m_uv_async_invite_accept;
  uv_async_t m_uv_async_quit_game;

  Client(int argc, char **argv, const char *host = CLIENT_DEFAULT_HOST, int port = CLIENT_DEFAULT_PORT);
  ~Client();

  void run();
  int run_qt_ui();
  void run_uv_loop();
  void set_is_connected(bool is_connected);
  void async_stop_uv_loop();
  void async_enter_lobby();
  void async_new_game_invite();
  void async_invite_accept();
  void async_quit_game();

  static void on_uv_timer_try_connect_timeout(uv_timer_t *timer);
  static void on_uv_tcp_server_connect(uv_connect_t *connect, int status);
  static void on_uv_tcp_connection_read(uv_stream_t *stream, long nread, const uv_buf_t *buf);
  static void on_uv_tcp_connection_alloc(uv_handle_t *handle, unsigned long size, uv_buf_t *buf);
  static void on_uv_tcp_connection_write(uv_write_t *write, int status);
  static void on_uv_tcp_connection_close(uv_handle_t *handle);

  static void on_uv_async_enter_lobby_awake(uv_async_t *handle);
  static void on_uv_async_stop_uv_loop_awake(uv_async_t *handle);
  static void on_uv_async_new_game_invite_awake(uv_async_t *handle);
  static void on_uv_async_invite_accept_awake(uv_async_t *handle);
  static void on_uv_async_quit_game_awake(uv_async_t *handle);
};

}; // namespace C4

#endif