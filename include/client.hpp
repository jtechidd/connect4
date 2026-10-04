#ifndef C4_CLIENT_HPP
#define C4_CLIENT_HPP

#include "common.hpp"
#include "game.hpp"
#include "ring_buffer.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_opengles2.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>

namespace C4 {
typedef enum {
  CLIENT_STATE_USERNAME,
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

    void handle_message(Message *msg);
    void handle_event(const EventPayload *event);
    void handle_event_server_connected(const EventServerConnected *sc);
    void handle_event_lobby_updated(const EventLobbyUpdated *lu);

    void send_message(Message *msg);
    void send_command_enter_lobby();
  };

  class UI {
  public:
    Client *m_client;
    char *m_glsl_version;
    SDL_WindowFlags m_sdl_window_flags;
    SDL_Window *m_sdl_window;
    SDL_GLContext m_sdl_gl_context;
    bool m_show_demo_window;
    bool m_show_another_window;
    bool m_done;
    SDL_Event m_sdl_event;
    float m_f = 0.0f;
    int m_counter = 0;

    UI(Client *client);
    ~UI();

    int run();
  };

  uv_loop_t *m_loop;
  char *m_host;
  int m_port;
  uv_tcp_t m_session;
  struct sockaddr_in m_server_addr;
  RingBuffer m_ring_buf;
  client_id_t m_client_id;
  uint64_t m_total_clients;
  client_state_t m_state;
  char m_username[65];
  Game m_game;
  MessageHandler m_msg_hdl;
  UI m_ui;
  std::thread m_thr_uv;
  std::mutex m_lock;
  uv_async_t m_keep_alive;
  bool m_is_connected;
  uv_timer_t m_try_connect;
  // From main thread (aka UI)
  uv_async_t m_stop;
  uv_async_t m_enter_lobby;

  Client(const char *host = "localhost", int port = 8080);
  ~Client();

  void run_uv();
  void run();
  void stop();
  void enter_lobby();

  static void on_try_connect(uv_timer_t *timer);
  static void on_connect(uv_connect_t *connect, int status);
  static void on_read(uv_stream_t *stream, long nread, const uv_buf_t *buf);
  static void on_alloc(uv_handle_t *handle, unsigned long size, uv_buf_t *buf);
  static void on_write(uv_write_t *write, int status);
  static void on_close(uv_handle_t *handle);

  static void async_enter_lobby(uv_async_t *handle);
  static void async_stop(uv_async_t *handle);
};
}; // namespace C4

#endif