#ifndef C4_CLIENT_DATA_HPP
#define C4_CLIENT_DATA_HPP

#include "common.hpp"

namespace C4 {
class ClientData {
public:
  client_id_t m_client_id;
  char m_username[USERNAME_MAX_SIZE + 1];
  ClientState m_state;
  game_id_t m_current_game_id;

  ClientData();
  ClientData(client_id_t client_id);
  ~ClientData();

  bool is_ready_for_new_game();
};
} // namespace C4

#endif