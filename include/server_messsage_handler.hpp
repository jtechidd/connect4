#ifndef C4_SERVER_MESSAGE_HANDLER_HPP
#define C4_SERVER_MESSAGE_HANDLER_HPP

#include "common.hpp"

namespace C4 {
class ServerMessageHandler {
public:
  Server *m_server;

  ServerMessageHandler(Server *server);
  ~ServerMessageHandler();

  void handle_message(client_id_t client_id, Message *msg);
  void handle_event(client_id_t client_id, const EventPayload *event_payload);
  void handle_command(client_id_t client_id, const CommandPayload *command_payload);
  void handle_command_enter_lobby(client_id_t client_id, const CommandEnterLobby *payload);
  void handle_command_new_game_invite(client_id_t client_id, const CommandNewGameInvite *payload);
  void handle_command_invite_accept(client_id_t client_id, const CommandInviteAccept *payload);

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
}; // namespace C4

#endif