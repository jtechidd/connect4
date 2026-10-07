#ifndef C4_CLIENT_HPP
#define C4_CLIENT_HPP

#include "common.hpp"
#include "game.hpp"
#include "ring_buffer.hpp"
#include <QtWidgets>

namespace C4 {

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
    void lobby_updated(const EventLobbyUpdated *lu);

    void on_btn_enter_lobby_clicked();
    void on_le_username_text_changed(const QString &text);
  };

  char *m_host;
  int m_port;

  uv_loop_t *m_uv_loop;
  std::thread m_thread_uv_loop;

  uv_timer_t m_uv_timer_try_connect;
  uv_tcp_t m_uv_tcp_connection;
  struct sockaddr_in m_server_address;
  RingBuffer m_ring_buffer;
  MessageHandler m_message_handler;

  bool m_is_connected;
  client_id_t m_client_id;
  uint64_t m_total_clients;
  ClientState m_state;
  char m_username[USERNAME_MAX_SIZE + 1];
  Game m_game;

  QApplication m_qt_app;
  QUI m_qt_ui;

  // UV async handles
  uv_async_t m_uv_async_keep_alive;
  // From main thread (UI)
  uv_async_t m_uv_async_stop_loop;
  uv_async_t m_uv_async_enter_lobby;

  Client(int argc, char **argv, const char *host = CLIENT_DEFAULT_HOST,
         int port = CLIENT_DEFAULT_PORT);
  ~Client();

  void run();
  int run_qt_ui();
  void run_uv_loop();
  void async_stop_uv_loop();
  void async_enter_lobby();

  static void on_uv_timer_try_connect_timeout(uv_timer_t *timer);
  static void on_uv_tcp_server_connect(uv_connect_t *connect, int status);
  static void on_uv_tcp_connection_read(uv_stream_t *stream, long nread,
                                        const uv_buf_t *buf);
  static void on_uv_tcp_connection_alloc(uv_handle_t *handle,
                                         unsigned long size, uv_buf_t *buf);
  static void on_uv_tcp_connection_write(uv_write_t *write, int status);
  static void on_uv_tcp_connection_close(uv_handle_t *handle);

  static void on_uv_async_enter_lobby_awake(uv_async_t *handle);
  static void on_uv_async_stop_uv_loop_awake(uv_async_t *handle);
};

}; // namespace C4

#endif