#include "client_data.hpp"
#include "message.pb.h"

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

}; // namespace C4