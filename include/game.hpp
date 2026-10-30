#ifndef C4_GAME_HPP
#define C4_GAME_HPP

#include "common.hpp"

namespace C4 {

constexpr uint8_t GAME_BOARD_WIDTH = 7;
constexpr uint8_t GAME_BOARD_HEIGHT = 6;
constexpr uint8_t GAME_CONNECT_COUNT = 4;
constexpr uint8_t GAME_CONNECT_AXES_SIZE = 4;
constexpr int8_t GAME_CONNECT_AXES[GAME_CONNECT_AXES_SIZE][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};
constexpr int8_t GAME_CONNECT_AXIS_SIGNS[2] = {1, -1};

class Game {
public:
  game_id_t m_id;
  client_id_t m_player1_id;
  client_id_t m_player2_id;
  bool m_player1_left;
  bool m_player2_left;

  uint8_t m_round;
  GameState m_state;
  GamePlayer m_turn;
  GameVerdict m_verdict;
  uint8_t m_connect_mask[GAME_BOARD_HEIGHT][GAME_BOARD_WIDTH];
  GamePlayer m_board[GAME_BOARD_HEIGHT][GAME_BOARD_WIDTH];

  Game();
  Game(game_id_t id, client_id_t player1_id, client_id_t player2_id);
  ~Game();

  client_id_t get_player_id_from_turn(GamePlayer turn);
  client_id_t get_current_turn_player_id();

  // Server only
  int start();
  int place(uint8_t j);
  int is_connect(uint8_t i, uint8_t j);
  int player_left(client_id_t client_id);

  // Client only
  client_id_t m_opponent_id;
  char m_opponent_username[USERNAME_MAX_SIZE + 1];
};

} // namespace C4

#endif