#include "game.hpp"
#include "message.pb.h"
#include "server.hpp"
#include "server_client_connection.hpp"
#include "server_messsage_handler.hpp"

namespace C4 {

ServerMessageHandler::ServerMessageHandler(Server *server) { m_server = server; }
ServerMessageHandler::~ServerMessageHandler() {}

void ServerMessageHandler::send_message(client_id_t client_id, Message *msg) {
  ServerClientConnection *client_conn = m_server->find_client_by_id(client_id);
  if (!client_conn)
    return;
  WriteRequest *write_req = new WriteRequest(msg);
  write_req->req.data = this;
  uv_write((uv_write_t *)write_req, (uv_stream_t *)&client_conn->m_uv_tcp_connection, &write_req->buf, 1, ServerClientConnection::on_uv_tcp_connection_write);
}

void ServerMessageHandler::broadcast_message(Message *msg) {
  for (auto [id, _] : m_server->m_clients_map) {
    send_message(id, msg);
  }
}

void ServerMessageHandler::emit_event_client_data_updated(client_id_t client_id) {
  ServerClientConnection *client = m_server->find_client_by_id(client_id);
  if (!client)
    return;
  Message msg;
  EventClientDataUpdated *payload = msg.mutable_event_payload()->mutable_event_client_data_updated();
  payload->set_client_id(client->m_client_data.m_client_id);
  payload->set_state(client->m_client_data.m_state);
  payload->set_current_game_id(client->m_client_data.m_current_game_id);
  payload->set_username(client->m_client_data.m_username);
  send_message(client_id, &msg);
}

void ServerMessageHandler::init_event_lobby_updated(Message *msg) {
  EventLobbyUpdated *payload = msg->mutable_event_payload()->mutable_event_lobby_updated();
  payload->set_total_clients(m_server->m_clients_map.size());
  payload->set_total_games(m_server->m_game_map.size());
  for (auto [_, game] : m_server->m_game_map) {
    GameInfo *game_info = payload->add_games();
    game_info->set_id(game->m_id);
    game_info->set_player1_id(game->m_player1_id);
    game_info->set_player2_id(game->m_player2_id);
    game_info->set_state(game->m_state);
  }
  for (auto [id, conn] : m_server->m_clients_map) {
    ClientInfo *client_info = payload->add_clients();
    client_info->set_id(conn->m_client_data.m_client_id);
    client_info->set_state(conn->m_client_data.m_state);
    client_info->set_username(conn->m_client_data.m_username);
  }
}

void ServerMessageHandler::broadcast_event_lobby_updated() {
  spdlog::debug("Broadcasting event [lobby_updated] to all clients.");
  Message msg;
  init_event_lobby_updated(&msg);
  broadcast_message(&msg);
}

void ServerMessageHandler::emit_event_lobby_updated(client_id_t client_id) {
  spdlog::debug("Emitting event [lobby_updated] to client with ID [{}].", client_id);
  Message msg;
  init_event_lobby_updated(&msg);
  send_message(client_id, &msg);
}

void ServerMessageHandler::emit_event_username_check_failed(client_id_t client_id) {
  spdlog::debug("Emitting event [username_check_failed] to client with ID [{}].", client_id);
  Message msg;
  msg.mutable_event_payload()->mutable_event_username_check_failed();
  send_message(client_id, &msg);
}

void ServerMessageHandler::emit_event_new_game_invite_received(client_id_t client_id, client_id_t inviter_client_id) {
  spdlog::debug("Emitting event [new_game_invite_received] to client with ID [{}].", client_id);
  ServerClientConnection *inviter_client = m_server->find_client_by_id(inviter_client_id);
  if (!inviter_client) {
    spdlog::debug("Inviter client with ID [{}] not found, skipping.", inviter_client_id);
    return;
  }
  Message msg;
  EventNewGameInviteReceived *payload = msg.mutable_event_payload()->mutable_event_new_game_invite_received();
  payload->set_inviter_client_id(inviter_client_id);
  payload->set_inviter_username(inviter_client->m_client_data.m_username);
  send_message(client_id, &msg);
}

void ServerMessageHandler::emit_event_game_updated(client_id_t client_id, game_id_t game_id) {
  spdlog::debug("Emitting event [game_updated] to client with ID [{}]", game_id);
  Game *game = m_server->find_game_by_id(game_id);
  if (!game)
    return;
  Message msg;
  EventGameUpdated *payload = msg.mutable_event_payload()->mutable_event_game_updated();
  payload->set_game_id(game->m_id);
  payload->set_player1_id(game->m_player1_id);
  payload->set_player2_id(game->m_player2_id);
  payload->set_state(game->m_state);
  payload->set_round(game->m_round);
  payload->set_turn(game->m_turn);
  payload->set_verdict(game->m_verdict);
  for (int i = 0; i < GAME_BOARD_HEIGHT; i++) {
    for (int j = 0; j < GAME_BOARD_WIDTH; j++) {
      payload->add_board(game->m_board[i][j]);
      payload->add_connect_mask(game->m_connect_mask[i][j]);
    }
  }
  client_id_t opponent_id;
  if (client_id == game->m_player1_id) {
    opponent_id = game->m_player2_id;
  } else if (client_id == game->m_player2_id) {
    opponent_id = game->m_player1_id;
  } else {
    return;
  }
  ServerClientConnection *opponent = m_server->find_client_by_id(opponent_id);
  if (!opponent)
    return;
  payload->set_opponent_client_id(opponent->m_client_data.m_client_id);
  payload->set_opponent_username(opponent->m_client_data.m_username);
  send_message(client_id, &msg);
}

void ServerMessageHandler::handle_message(client_id_t client_id, Message *msg) {
  switch (msg->payload_type_case()) {
  case Message::kEventPayload:
    handle_event(client_id, &msg->event_payload());
    break;
  case Message::kCommandPayload:
    handle_command(client_id, &msg->command_payload());
  case Message::PAYLOAD_TYPE_NOT_SET:
  default:
    break;
  }
}

void ServerMessageHandler::handle_event(client_id_t client_id, const EventPayload *event_payload) {
  switch (event_payload->payload_case()) {
  default:
    break;
  }
}

void ServerMessageHandler::handle_command(client_id_t client_id, const CommandPayload *command_payload) {
  switch (command_payload->payload_case()) {
  case CommandPayload::kCommandEnterLobby:
    handle_command_enter_lobby(client_id, &command_payload->command_enter_lobby());
    break;
  case CommandPayload::kCommandNewGameInvite:
    handle_command_new_game_invite(client_id, &command_payload->command_new_game_invite());
    break;
  case CommandPayload::kCommandInviteAccept:
    handle_command_invite_accept(client_id, &command_payload->command_invite_accept());
    break;
  case CommandPayload::kCommandPlayerMakeMove:
    handle_command_player_make_move(client_id, &command_payload->command_player_make_move());
    break;
  case CommandPayload::kCommandPlayerQuitGame:
    handle_command_player_quit_game(client_id, &command_payload->command_player_quit_game());
    break;
  default:
    break;
  }
}

void ServerMessageHandler::handle_command_enter_lobby(client_id_t client_id, const CommandEnterLobby *payload) {
  spdlog::info("Handling command [enter_lobby] from client with ID [{}].", client_id);
  for (auto [_, client_conn] : m_server->m_clients_map) {
    if (!strcmp(client_conn->m_client_data.m_username, payload->username().c_str())) {
      spdlog::warn("Username [{}] is already taken, rejecting.", client_conn->m_client_data.m_username);
      emit_event_username_check_failed(client_id);
      return;
    }
  }
  ServerClientConnection *client_conn = m_server->find_client_by_id(client_id);
  assert(client_conn != NULL);
  assert(payload->username().length() <= USERNAME_MAX_SIZE);
  client_conn->m_client_data.enter_lobby(payload);
  emit_event_client_data_updated(client_id);
  emit_event_lobby_updated(client_id);
}

void ServerMessageHandler::handle_command_new_game_invite(client_id_t client_id, const CommandNewGameInvite *payload) {
  spdlog::info("Handling command [new_game_invite] from client with ID [{}].", client_id);
  emit_event_new_game_invite_received(payload->client_id(), client_id);
}

void ServerMessageHandler::handle_command_invite_accept(client_id_t client_id, const CommandInviteAccept *payload) {
  spdlog::info("Handling command [invite_accept] from client with ID [{}]", client_id);
  client_id_t client1_id = payload->inviter_client_id();
  client_id_t client2_id = client_id;
  m_server->create_new_game(client1_id, client2_id);
}

void ServerMessageHandler::handle_command_player_make_move(client_id_t client_id, const CommandPlayerMakeMove *payload) {
  spdlog::info("Handling command [player_make_move] from client with ID [{}]", client_id);
  if (client_id != payload->player_id()) {
    spdlog::warn("Client ID [{}] and player ID [{}] mismatch, skipping.");
    return;
  }
  Game *game = m_server->find_game_by_id(payload->game_id());
  if (game == NULL) {
    spdlog::warn("Game with ID [{}] not found, skipping.", payload->game_id());
    return;
  }
  if (game->place(payload->played_column()) < 0) {
    spdlog::warn("Cannot play on column [{}], skipping.", payload->played_column());
    return;
  }

  emit_event_game_updated(game->m_player1_id, game->m_id);
  emit_event_game_updated(game->m_player2_id, game->m_id);
}

void ServerMessageHandler::handle_command_player_quit_game(client_id_t client_id, const CommandPlayerQuitGame *payload) {
  spdlog::info("Handling command [player_quit_game] from client with ID [{}]", client_id);
  if (client_id != payload->player_id()) {
    spdlog::warn("Client ID [{}] and player ID [{}] mismatch, skipping.");
    return;
  }
  ServerClientConnection *client = m_server->find_client_by_id(client_id);
  if (!(client && client->m_client_data.m_client_id == client_id)) {
    spdlog::warn("Client with ID [{}] not found or mismatch, skipping.", client_id);
    return;
  }
  if (payload->game_id() != client->m_client_data.m_current_game_id) {
    spdlog::warn("Payload game ID [{}] and client current game ID [{}] mismatch, skipping.", payload->game_id(), client->m_client_data.m_current_game_id);
    return;
  }
  Game *game = m_server->find_game_by_id(payload->game_id());
  if (game == NULL) {
    spdlog::warn("Game with ID [{}] not found, skipping.", payload->game_id());
    return;
  }

  client->m_client_data.quit_game();
  emit_event_client_data_updated(client_id);

  game->player_left(client_id);
  if (game->m_player1_left && game->m_player2_left) {
    m_server->delete_game(game->m_id);
  } else if (game->m_player1_left) {
    emit_event_game_updated(game->m_player2_id, game->m_id);
  } else if (game->m_player2_left) {
    emit_event_game_updated(game->m_player1_id, game->m_id);
  }
}

}; // namespace C4