#include "game.hpp"
#include "message.pb.h"

namespace C4 {

Game::Game() {
  m_id = 0;
  m_player1_id = 0;
  m_player2_id = 0;
  m_player1_left = false;
  m_player2_left = false;
  m_opponent_id = 0;
  m_round = 0;
  m_state = GAME_STATE_WAITING;
  m_turn = GAME_PLAYER_1;
  m_verdict = GAME_VERDICT_UNDECIDED;
  memset(m_opponent_username, 0, sizeof(m_opponent_username));
  memset(m_connect_mask, 0, sizeof(m_connect_mask));
  memset(m_board, 0, sizeof(m_board));
};
Game::Game(game_id_t id, client_id_t player1_id, client_id_t player2_id) : Game() {
  m_id = id;
  m_player1_id = player1_id;
  m_player2_id = player2_id;
}
Game::~Game() {};

int Game::start() {
  if (!(m_player1_id && m_player2_id))
    return -1;
  m_state = GAME_STATE_IN_GAME;
  m_turn = GAME_PLAYER_1;
  m_round = 0;
  memset(m_board, 0, sizeof(m_board));
  return 0;
}

int Game::place(uint8_t j) {
  if (m_state != GAME_STATE_IN_GAME)
    return -1;
  if (j >= GAME_BOARD_WIDTH)
    return -1;
  int i = -1;
  for (i = 0; i < GAME_BOARD_HEIGHT; i++) {
    if (m_board[i][j]) {
      break;
    }
  }
  i--;
  if (i == -1) {
    return -1;
  }
  m_board[i][j] = m_turn;
  m_round++;
  if (is_connect(i, j)) {
    m_state = GAME_STATE_VERDICT;
    if (m_turn == GAME_PLAYER_1)
      m_verdict = GAME_VERDICT_PLAYER1_WIN;
    else if (m_turn == GAME_PLAYER_2)
      m_verdict = GAME_VERDICT_PLAYER2_WIN;
  } else if (m_round == GAME_BOARD_WIDTH * GAME_BOARD_HEIGHT) {
    m_state = GAME_STATE_VERDICT;
    m_verdict = GAME_VERDICT_TIE;
  }
  if (m_turn == GAME_PLAYER_1)
    m_turn = GAME_PLAYER_2;
  else if (m_turn == GAME_PLAYER_2)
    m_turn = GAME_PLAYER_1;
  return 0;
};

int Game::is_connect(uint8_t i, uint8_t j) {
  int8_t k, di, dj, s, ti, tj, c;
  for (k = 0; k < GAME_CONNECT_AXES_SIZE; k++) {
    memset(m_connect_mask, 0, sizeof(m_connect_mask));
    m_connect_mask[i][j] = 1;
    c = 1;
    for (int8_t sign : GAME_CONNECT_AXIS_SIGNS) {
      di = sign * GAME_CONNECT_AXES[k][0];
      dj = sign * GAME_CONNECT_AXES[k][1];
      for (s = 1; s <= GAME_CONNECT_COUNT - 1; s++) {
        ti = i + s * di;
        tj = j + s * dj;
        if (ti < 0 || ti >= GAME_BOARD_HEIGHT || tj < 0 || tj >= GAME_BOARD_WIDTH)
          break;
        if (m_board[ti][tj] != m_turn)
          break;
        m_connect_mask[ti][tj] = 1;
        c++;
      }
    }
    if (c >= GAME_CONNECT_COUNT) {
      return 1;
    }
  }
  return 0;
}

int Game::player_left(client_id_t player_id) {
  if (m_state == GAME_STATE_IN_GAME) {
    m_state = GAME_STATE_VERDICT;
    if (player_id == m_player1_id) {
      m_verdict = GAME_VERDICT_PLAYER2_WIN;
    } else if (player_id == m_player2_id) {
      m_verdict = GAME_VERDICT_PLAYER1_WIN;
    }
  }
  if (player_id == m_player1_id) {
    m_player1_left = true;
  } else if (player_id == m_player2_id) {
    m_player2_left = true;
  }
  return 0;
}

client_id_t Game::get_player_id_from_turn(GamePlayer turn) {
  if (turn == GAME_PLAYER_1)
    return m_player1_id;
  if (turn == GAME_PLAYER_2)
    return m_player2_id;
  return 0;
}

client_id_t Game::get_current_turn_player_id() { return get_player_id_from_turn(m_turn); }

}; // namespace C4