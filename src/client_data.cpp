#include "client_data.hpp"
#include "game.hpp"

namespace C4 {

ClientData::ClientData() {
  m_client_id = 0;
  m_state = CLIENT_STATE_USERNAME;
  m_current_game_id = 0;
  memset(m_username, 0, sizeof(m_username));
}
ClientData::ClientData(client_id_t client_id) : ClientData() { m_client_id = client_id; }
ClientData::~ClientData() {}

bool ClientData::is_ready_for_new_game() { return m_state == CLIENT_STATE_LOBBY && m_current_game_id == 0; }

void ClientData::enter_lobby(const CommandEnterLobby *payload) {
  snprintf(m_username, sizeof(m_username), "%s", payload->username().c_str());
  m_state = CLIENT_STATE_LOBBY;
}

void ClientData::quit_game() {
  m_current_game_id = 0;
  m_state = CLIENT_STATE_LOBBY;
}

void ClientData::join_game(Game *game) {
  m_current_game_id = game->m_id;
  m_state = CLIENT_STATE_IN_GAME;
}

}; // namespace C4