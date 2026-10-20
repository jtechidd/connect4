#ifndef C4_CLIENT_MESSAGE_HANDLER_HPP
#define C4_CLIENT_MESSAGE_HANDLER_HPP

#include "common.hpp"
#include "message.pb.h"

namespace C4 {
class ClientMessageHandler {
public:
  Client *m_client;

  ClientMessageHandler(Client *client);
  ~ClientMessageHandler();

  void handle_message(C4::Message *msg);
  void handle_event(const EventPayload *event_payload);
  void handle_event_client_data_updated(const EventClientDataUpdated *payload);
  void handle_event_username_check_failed(const EventUsernameCheckFailed *payload);
  void handle_event_lobby_updated(const EventLobbyUpdated *payload);
  void handle_event_new_game_invite_received(const EventNewGameInviteReceived *payload);
  void handle_event_game_updated(const EventGameUpdated *payload);

  void send_message(Message *msg);
  void send_command_enter_lobby();
  void send_command_new_game_invite();
  void send_command_invite_accept();
  void send_command_player_make_move(int col);
  void send_command_quit_game();
};
}; // namespace C4

#endif