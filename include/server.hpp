#ifndef C4_SERVER_HPP
#define C4_SERVER_HPP

#include "common.hpp"
#include "message.pb.h"
#include "ring_buffer.hpp"

namespace C4 {
class Server {
public:
  class ClientConnection {
  public:
    uv_loop_t *m_uv_loop;
    Server *m_server;
    client_id_t m_id;
    uv_tcp_t m_uv_tcp_connection;
    char m_username[USERNAME_MAX_SIZE + 1];
    ClientState m_state;
    RingBuffer m_ring_buffer;
    game_id_t m_current_game_id;
    std::deque<NewGameInvite> m_new_game_invite_queue;

    ClientConnection(uv_loop_t *loop, Server *server = nullptr, client_id_t id = ++g_client_cid);
    ~ClientConnection();

    void run();

    void emit_event_server_connected();

    // UV callbacks
    static void on_uv_tcp_connection_alloc(uv_handle_t *handle, unsigned long size, uv_buf_t *buf);
    static void on_uv_tcp_connection_read(uv_stream_t *stream, long nread, const uv_buf_t *buf);
    static void on_uv_tcp_connection_close(uv_handle_t *handle);
    static void on_uv_tcp_connection_write(uv_write_t *write, int status);
  };

  class MessageHandler {
  public:
    Server *m_server;

    MessageHandler(Server *server);
    ~MessageHandler();

    void handle_message(client_id_t client_id, Message *msg);
    void handle_event(client_id_t client_id, const EventPayload *event_payload);
    void handle_command(client_id_t client_id, const CommandPayload *command_payload);
    void handle_command_enter_lobby(client_id_t client_id, const CommandEnterLobby *el);
    void handle_command_new_game_invite(client_id_t client_id, const CommandNewGameInvite *ngr);
    void handle_command_invite_accept(client_id_t client_id, const CommandInviteAccept *ia);

    void send_message(client_id_t client_id, Message *msg);
    void broadcast_message(Message *msg);
    void emit_event_server_connected(client_id_t client_id);
    void init_event_lobby_updated(Message *msg);
    void emit_event_lobby_updated(client_id_t client_id);
    void broadcast_event_lobby_updated();
    void emit_event_username_check_failed(client_id_t client_id);
    void emit_event_lobby_entered(client_id_t client_id);
    void emit_event_new_game_invite_received(client_id_t client_id, client_id_t invited_client_id);
    void emit_event_new_game_joined(client_id_t client_id, game_id_t game_id, client_id_t opp_client_id);
  };

  uv_loop_t *m_uv_loop;
  int m_port;
  int m_backlog;
  uv_tcp_t m_uv_tcp_server;
  struct sockaddr_in m_server_addr;
  client_id_t m_client_cid;
  std::map<client_id_t, ClientConnection *> m_clients_map;
  game_id_t m_game_cid;
  std::map<game_id_t, Game *> m_game_map;
  MessageHandler m_message_handler;
  uv_timer_t m_uv_timer_broadcast_event_lobby_updated;

  Server(int port = 8080, int backlog = 128);
  ~Server();

  void run();

  void disconnect(client_id_t client_id);
  void create_new_game(client_id_t client_id);
  void join_game(client_id_t client_id, game_id_t game_id);

  // UV callbacks
  static void on_uv_tcp_server_connection(uv_stream_t *stream, int status);
  static void on_uv_timer_broadcast_event_lobby_updated_timeout(uv_timer_t *timer);
};
} // namespace C4

#endif