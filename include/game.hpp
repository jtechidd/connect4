#ifndef C4_GAME_HPP
#define C4_GAME_HPP

#include "common.hpp"

namespace C4 {

typedef enum {
  GAME_PLAYER_1 = 1,
  GAME_PLAYER_2,
} game_player_t;

typedef enum {
  GAME_VERDICT_UNDECIDED = 1,
  GAME_VERDICT_PLAYER_1_WIN,
  GAME_VERDICT_PLAYER_2_WIN,
  GAME_VERDICT_TIE,
} game_verdict_t;

class Game {
public:
  game_id_t m_id;
  client_id_t m_player1_id;
  client_id_t m_player2_id;

  // Client
  client_id_t m_opp_id;
  char m_opp_username[USERNAME_MAX_SIZE + 1];

  uint8_t m_round;
  GameState m_state;
  game_player_t m_turn;
  game_verdict_t m_verdict;
  game_player_t m_board[6][7];

  Game();
  Game(game_id_t id, client_id_t player1_id, client_id_t player2_id);
  ~Game();

  // server side
  int start();
  int place(uint8_t j);
  int is_connect(uint8_t i, uint8_t j);

  client_id_t get_player_id_from_turn(game_player_t turn);
  client_id_t get_current_turn_player_id();
};

} // namespace C4

#endif