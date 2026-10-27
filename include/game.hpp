#ifndef C4_GAME_HPP
#define C4_GAME_HPP

#include "common.hpp"

namespace C4 {

class Game {
public:
  game_id_t m_id;
  client_id_t m_player1_id;
  client_id_t m_player2_id;
  bool m_player1_left;
  bool m_player2_left;

  // Client
  client_id_t m_opponent_id;
  char m_opponent_username[USERNAME_MAX_SIZE + 1];

  uint8_t m_round;
  GameState m_state;
  GamePlayer m_turn;
  GameVerdict m_verdict;
  uint8_t m_connect_mask[6][7];
  GamePlayer m_board[6][7];

  Game();
  Game(game_id_t id, client_id_t player1_id, client_id_t player2_id);
  ~Game();

  // server side
  int start();
  int place(uint8_t j);
  int is_connect(uint8_t i, uint8_t j);
  int player_left(client_id_t client_id);

  client_id_t get_player_id_from_turn(GamePlayer turn);
  client_id_t get_current_turn_player_id();
};

} // namespace C4

#endif