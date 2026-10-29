#ifndef C4_CLIENT_UI_HPP
#define C4_CLIENT_UI_HPP

#include "common.hpp"
#include <QtWidgets>

namespace C4 {

constexpr const char *CLIENT_UI_USERNAME = "Username";
constexpr const char *CLIENT_UI_SERVER_DISCONNECTED = "Server disconnected";
constexpr const char *CLIENT_UI_SERVER_CONNECTED = "Server connected";
constexpr const char *CLIENT_UI_ENTER_LOBBY = "Enter Lobby";
constexpr const char *CLIENT_UI_ONLINE_PLAYERS = "Online players:";
constexpr const char *CLIENT_UI_PLAYER = "player";
constexpr const char *CLIENT_UI_GOT_INVITED = "You got invited from %1";
constexpr const char *CLIENT_UI_ACCEPT = "Accept";
constexpr const char *CLIENT_UI_DECLINE = "Decline";
constexpr const char *CLIENT_UI_NEW_GAME = "New Game";
constexpr const char *CLIENT_UI_NEW_GAME_WITH = "New Game With %1";
constexpr const char *CLIENT_UI_QUIT_GAME = "Quit Game";
constexpr const char *CLIENT_UI_OPPONENT = "Opponent";
constexpr const char *CLIENT_UI_YOU = "You";
constexpr const char *CLIENT_UI_QUIT_GAME_MSGBOX_TITLE = "Quit Game?";
constexpr const char *CLIENT_UI_QUIT_GAME_MSGBOX_TEXT = "Are you sure want to quit the game?";
constexpr const char *CLIENT_UI_USERNAME_CHECK_FAILED_MSGBOX_TITLE = "Cannot use this username";
constexpr const char *CLIENT_UI_USERNAME_CHECK_FAILED_MSGBOX_TEXT = "This username is already taken, please try another.";
constexpr const char *CLIENT_UI_WINNER = "%1 (winner)";
constexpr const char *CLIENT_UI_WINDOW_TITLE = "Connect4";
constexpr const char *CLIENT_UI_WINDOW_TITLE_WITH_USERNAME = "Connect4 (%1)";
constexpr const int CLIENT_UI_WINDOW_MARGIN = 8;
constexpr const int CLIENT_UI_GAME_TILE_MARGINS = 5;
constexpr const Qt::GlobalColor CLIENT_UI_GAME_BOARD_COLOR = Qt::gray;
constexpr const Qt::GlobalColor CLIENT_UI_GAME_SLOT_COLOR_ENABLE = Qt::white;
constexpr const Qt::GlobalColor CLIENT_UI_GAME_SLOT_COLOR_DISABLE = Qt::lightGray;
constexpr const Qt::GlobalColor CLIENT_UI_GAME_CONNECT_TOKEN_BORDER_COLOR = Qt::white;
constexpr const int CLIENT_UI_GAME_CONNECT_TOKEN_BORDER_WIDTH = 4;
constexpr const Qt::GlobalColor CLIENT_UI_GAME_TOKEN_COLOR_PLAYER = Qt::blue;
constexpr const Qt::GlobalColor CLIENT_UI_GAME_TOKEN_COLOR_OPPONENT = Qt::red;

class QClientWidget : public QWidget {
public:
  Client *m_client;

  QClientWidget(Client *client, QWidget *parent = nullptr);
};

class QClientVBoxLayout : public QVBoxLayout {
public:
  QClientVBoxLayout(QWidget *parent = nullptr);
  QClientVBoxLayout(int margin, QWidget *parent = nullptr);
};
class QClientHBoxLayout : public QHBoxLayout {
public:
  QClientHBoxLayout(QWidget *parent = nullptr);
  QClientHBoxLayout(int margin, QWidget *parent = nullptr);
};
class QClientFormLayout : public QFormLayout {
public:
  QClientFormLayout(QWidget *parent = nullptr);
  QClientFormLayout(int margin, QWidget *parent = nullptr);
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