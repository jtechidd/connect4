#ifndef C4_CLIENT_UI_HPP
#define C4_CLIENT_UI_HPP

#include "common.hpp"
#include <QtWidgets>

namespace C4 {

class QClientWidget : public QWidget {
public:
  Client *m_client;

  QClientWidget(Client *client, QWidget *parent = nullptr);
};

class QGameBoard : public QClientWidget {
public:
  QGameBoard(Client *client, QWidget *parent = nullptr);

  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
};

class QPageUsername : public QClientWidget {
public:
  QLineEdit m_le_username;
  QLabel m_lb_server_status;
  QPushButton m_btn_enter_lobby;

  QPageUsername(Client *client, QWidget *parent = nullptr);

  void on_le_username_text_changed(const QString &text);
  void on_btn_enter_lobby_clicked();
  void update_server_connection();
};

class QOnlinePlayers : public QClientWidget {
public:
  QListWidget m_lw_online_players;

  QOnlinePlayers(Client *client, QWidget *parent = nullptr);

  void on_lw_online_players_item_clicked(QListWidgetItem *item);

  void update_online_client_infos();
};

class QInvite : public QClientWidget {
public:
  QLabel m_lb_invite_text;
  QPushButton m_btn_invite_accept;
  QPushButton m_btn_invite_decline;

  QInvite(Client *client, QWidget *parent = nullptr);

  void on_btn_invite_accept_clicked();
  void on_btn_invite_decline_clicked();

  void update_invite();
};

class QGameMenu : public QClientWidget {
public:
  QPushButton m_btn_new_game;
  QPushButton m_btn_new_game_invite;
  QInvite m_invite;

  QGameMenu(Client *client, QWidget *parent = nullptr);
  void on_btn_new_game_invite_clicked();

  void update_selected_client();
  void update_invite();
};

class QGame : public QClientWidget {
public:
  QPushButton m_btn_game_quit;
  QLabel m_lb_game_opponent;
  QGameBoard m_game_board;
  QLabel m_lb_game_player;

  QGame(Client *client, QWidget *parent = nullptr);

  void on_btn_game_quit_clicked();
  void update_game();
};

class QGameArea : public QClientWidget {
public:
  QStackedWidget m_sw_game;
  QGameMenu m_game_menu;
  QGame m_game;

  QGameArea(Client *client, QWidget *parent = nullptr);

  void update_client_data();
};

class QPageLobby : public QClientWidget {
public:
  QOnlinePlayers m_online_players;
  QGameArea m_game_area;

  QPageLobby(Client *client, QWidget *parent = nullptr);
};

class QClientUI : public QClientWidget {
public:
  QClientUI(Client *client, QWidget *parent = nullptr);

  QStackedWidget m_sw_pages;
  QPageUsername m_page_username;
  QPageLobby m_page_lobby;

  void username_check_failed();
  void update_server_connection();
  void update_client_data();
  void update_online_client_infos();
  void update_invite();
  void update_game();
};
}; // namespace C4

#endif