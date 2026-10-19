#ifndef C4_CLIENT_UI_HPP
#define C4_CLIENT_UI_HPP

#include "common.hpp"
#include <QtWidgets>

namespace C4 {

class Board : public QWidget {
public:
  Board(QWidget *parent = nullptr);
  void paintEvent(QPaintEvent *event) override;
};

class ClientUI : public QWidget {
public:
  ClientUI(Client *client, QWidget *parent = nullptr);

  Client *m_client;

  QStackedWidget *m_sw_pages;

  QLineEdit *m_le_username;
  QLabel *m_lb_server_status;
  QPushButton *m_btn_enter_lobby;

  QListWidget *m_lw_online_players;

  QPushButton *m_btn_new_game_invite;

  QWidget *m_wg_invite;
  QLabel *m_lb_invite_text;
  QPushButton *m_btn_invite_accept;
  QPushButton *m_btn_invite_decline;

  QStackedWidget *m_sw_game;

  QLabel *m_lb_game_opp_side;
  QLabel *m_lb_game_player_side;
  QPushButton *m_btn_board_cell[6][7];

  void update_server_connection(bool is_connected);
  void username_check_failed();
  void lobby_entered();
  void lobby_updated(const EventLobbyUpdated *payload);
  void new_game_joined(const EventNewGameJoined *payload);
  void update_invite();

  void on_btn_enter_lobby_clicked();
  void on_le_username_text_changed(const QString &text);
  void on_lw_online_players_item_clicked(QListWidgetItem *item);
  void on_btn_new_game_invite_clicked();
  void on_btn_invite_accept_clicked();
  void on_btn_invite_decline_clicked();
};
}; // namespace C4

#endif