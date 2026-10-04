#ifndef C4_SERVER_HPP
#define C4_SERVER_HPP

#include "common.hpp"
#include "message.pb.h"
#include "ring_buffer.hpp"

namespace C4 {
class Server {
public:
  class Session {
  public:
    uv_loop_t *m_loop;
    Server *m_server;
    session_id_t m_id;
    uv_tcp_t m_session;
    char m_username[65];
    RingBuffer m_ring_buf;
    game_id_t m_curr_game_id;

    Session(uv_loop_t *loop = g_loop, Server *server = nullptr,
            session_id_t id = ++g_session_cid);
    ~Session();

    void run();

    void emit_event_server_connected();

    // UV callbacks
    static void on_alloc(uv_handle_t *handle, unsigned long size,
                         uv_buf_t *buf);
    static void on_read(uv_stream_t *stream, long nread, const uv_buf_t *buf);
    static void on_close(uv_handle_t *handle);
    static void on_write(uv_write_t *req, int status);
  };

  class MessageHandler {
  public:
    Server *m_server;

    MessageHandler(Server *server);
    ~MessageHandler();

    void handle_message(session_id_t session_id, Message *msg);
    void handle_event(session_id_t session_id,
                      const EventPayload *event_payload);
    void handle_command(session_id_t session_id,
                        const CommandPayload *command_payload);
    void handle_command_enter_lobby(session_id_t session_id,
                                    const CommandEnterLobby *el);

    void send_message(session_id_t session_id, Message *msg);
    void broadcast_message(Message *msg);
    void emit_event_server_connected(session_id_t session_id);
    void broadcast_event_lobby_updated();
  };

  uv_loop_t *m_loop;                                // Main UV loop
  int m_port;                                       // Server port
  int m_backlog;                                    // Server backlog
  uv_tcp_t m_server;                                // UV TCP Server
  struct sockaddr_in m_server_addr;                 // Server address info
  session_id_t m_session_cid;                       // Session counting ID
  std::map<session_id_t, Session *> m_sessions_map; // Session map
  game_id_t m_game_cid;                             // Game counting ID
  std::map<game_id_t, Game *> m_game_map;           // Game map
  MessageHandler m_msg_hdl;                         // Message handler

  Server(int port = 8080, int backlog = 128);
  ~Server();

  void run();

  void disconnect(session_id_t session_id);
  void create_new_game(session_id_t session_id);
  void join_game(session_id_t session_id, game_id_t game_id);

  // UV callbacks
  static void on_connection(uv_stream_t *stream, int status);
};
} // namespace C4

#endif