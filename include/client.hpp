#ifndef C4_CLIENT_HPP
#define C4_CLIENT_HPP

#include "common.hpp"
#include "game.hpp"
#include "message.pb.h"
#include "ring_buffer.hpp"
#include <QtWidgets>

namespace C4 {

typedef enum {
  CLIENT_STATE_USERNAME,
  CLIENT_STATE_LOBBY,
  CLIENT_STATE_IN_GAME,
} client_state_t;

extern const char *CLIENT_DEFAULT_HOST;
extern const int CLIENT_DEFAULT_PORT;

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
    void
    handle_event_username_check_failed(const EventUsernameCheckFailed *ucf);
    void handle_event_lobby_entered(const EventLobbyEntered *le);
    void handle_event_lobby_updated(const EventLobbyUpdated *lu);

    void send_message(Message *msg);
    void send_command_enter_lobby();
  };

  class QUI : public QWidget {
  public:
    QUI(Client *client, QWidget *parent = nullptr);

    Client *m_client;

    QStackedWidget *m_sw_pages;

    QLineEdit *m_le_username;
    QLabel *m_lb_server_status;
    QPushButton *m_btn_enter_lobby;

    QListWidget *m_lw_online_players;

    void update_server_connection(bool is_connected);
    void username_check_failed();
    void lobby_entered();

    void on_btn_enter_lobby_clicked();
    void on_le_username_text_changed(const QString &text);
  };

  char *m_host;
  int m_port;

  uv_loop_t *m_loop;
  std::thread m_thread_uv;

  uv_timer_t m_try_connect;
  uv_tcp_t m_connection;
  struct sockaddr_in m_server_addr;
  RingBuffer m_ring_buf;
  MessageHandler m_msg_hdl;

  bool m_is_connected;
  client_id_t m_client_id;
  uint64_t m_total_clients;
  client_state_t m_state;
  char m_username[USERNAME_MAX_SIZE + 1];
  Game m_game;

  QApplication m_qapp;
  QUI m_qui;

  // UV async handles
  uv_async_t m_keep_alive;
  // From main thread (UI)
  uv_async_t m_stop;
  uv_async_t m_enter_lobby;

  Client(int argc, char **argv, const char *host = CLIENT_DEFAULT_HOST,
         int port = CLIENT_DEFAULT_PORT);
  ~Client();

  void run();
  int run_ui();
  void run_uv();
  void stop_async();
  void enter_lobby_async();

  static void on_try_connect(uv_timer_t *timer);
  static void on_connect(uv_connect_t *connect, int status);
  static void on_read(uv_stream_t *stream, long nread, const uv_buf_t *buf);
  static void on_alloc(uv_handle_t *handle, unsigned long size, uv_buf_t *buf);
  static void on_write(uv_write_t *write, int status);
  static void on_close(uv_handle_t *handle);

  static void enter_lobby_async_cb(uv_async_t *handle);
  static void stop_async_cb(uv_async_t *handle);
};

}; // namespace C4

#endif