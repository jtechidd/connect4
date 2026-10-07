#include "game.hpp"

namespace C4 {

Game::Game() {
  m_id = 0;
  m_p1_id = 0;
  m_p2_id = 0;
  m_round = 0;
  m_state = GAME_STATE_WAITING;
  m_turn = GAME_PLAYER_1;
  m_verdict = GAME_VERDICT_UNDECIDED;
  memset(m_board, 0, sizeof(m_board));
};
Game::Game(uint64_t id, client_id_t p1_id) : Game() {
  m_id = id;
  m_p1_id = p1_id;
}
Game::~Game() {};

int Game::start() {
  if (!(m_p1_id && m_p2_id))
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
  if (j >= 7)
    return -1;
  int i = -1;
  for (i = 0; i < 6; i++) {
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
      m_verdict = GAME_VERDICT_PLAYER_1_WIN;
    else if (m_turn == GAME_PLAYER_2)
      m_verdict = GAME_VERDICT_PLAYER_2_WIN;
  } else if (m_round == 42) {
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
  int8_t dir[4][2] = {{0, -1}, {-1, -1}, {-1, 0}, {0, 1}};
  for (k = 0; k < 4; k++) {
    di = dir[k][0];
    dj = dir[k][1];
    c = 1;
    for (s = 1; s <= 3; s++) {
      ti = i + s * di;
      tj = j + s * dj;
      if (ti < 0 || ti >= 6 || tj < 0 || tj >= 7)
        break;
      if (m_board[ti][tj] != m_turn)
        break;
      c++;
    }
    for (s = -1; s >= -3; s++) {
      ti = i + s * di;
      tj = j + s * dj;
      if (ti < 0 || ti >= 6 || tj < 0 || tj >= 7)
        break;
      if (m_board[ti][tj] != m_turn)
        break;
      c++;
    }
    if (c >= 4) {
      return 1;
    }
  }
  return 0;
}

}; // namespace C4