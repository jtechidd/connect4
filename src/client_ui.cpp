#include "client_ui.hpp"
#include "client.hpp"
#include "game.hpp"

namespace C4 {

QClientWidget::QClientWidget(Client *client, QWidget *parent) : QWidget(parent) { m_client = client; }

QClientVBoxLayout::QClientVBoxLayout(int margin, QWidget *parent) : QVBoxLayout(parent) { setContentsMargins(margin, margin, margin, margin); }
QClientVBoxLayout::QClientVBoxLayout(QWidget *parent) : QClientVBoxLayout(0, parent) {}
QClientHBoxLayout::QClientHBoxLayout(int margin, QWidget *parent) : QHBoxLayout(parent) { setContentsMargins(margin, margin, margin, margin); }
QClientHBoxLayout::QClientHBoxLayout(QWidget *parent) : QClientHBoxLayout(0, parent) {}
QClientFormLayout::QClientFormLayout(int margin, QWidget *parent) : QFormLayout(parent) { setContentsMargins(margin, margin, margin, margin); }
QClientFormLayout::QClientFormLayout(QWidget *parent) : C4::QClientFormLayout(0, parent) {}

QPageUsername::QPageUsername(Client *client, QWidget *parent) : QClientWidget(client, parent) {
  QClientVBoxLayout *vbl_username = new QClientVBoxLayout;
  QClientFormLayout *fl_username = new QClientFormLayout;
  m_le_username.setMaxLength(USERNAME_MAX_SIZE);
  connect(&m_le_username, &QLineEdit::textChanged, this, &QPageUsername::on_le_username_text_changed);
  fl_username->addRow(new QLabel(CLIENT_UI_USERNAME), &m_le_username);
  vbl_username->addLayout(fl_username);
  m_lb_server_status.setText(CLIENT_UI_SERVER_DISCONNECTED);
  vbl_username->addWidget(&m_lb_server_status);
  QClientHBoxLayout *hbl_btn = new QClientHBoxLayout;
  m_btn_enter_lobby.setText(CLIENT_UI_ENTER_LOBBY);
  m_btn_enter_lobby.setEnabled(false);
  hbl_btn->addWidget(&m_btn_enter_lobby);
  hbl_btn->addStretch(1);
  vbl_username->addLayout(hbl_btn);
  vbl_username->addStretch(1);
  connect(&m_btn_enter_lobby, &QPushButton::clicked, this, &QPageUsername::on_btn_enter_lobby_clicked);

  setLayout(vbl_username);
}

void QPageUsername::on_btn_enter_lobby_clicked() { m_client->async_enter_lobby(); }

void QPageUsername::on_le_username_text_changed(const QString &text) {
  snprintf(m_client->m_username_input, sizeof(m_client->m_username_input), "%s", text.toUtf8().constData());
}

void QPageUsername::update_server_connection() {
  if (m_client->m_is_connected) {
    m_lb_server_status.setText(CLIENT_UI_SERVER_CONNECTED);
    m_btn_enter_lobby.setEnabled(true);
  } else {
    m_lb_server_status.setText(CLIENT_UI_SERVER_DISCONNECTED);
    m_btn_enter_lobby.setEnabled(false);
  }
}

QOnlinePlayers::QOnlinePlayers(Client *client, QWidget *parent) : QClientWidget(client, parent) {
  QClientVBoxLayout *vbl_online_players = new QClientVBoxLayout;
  vbl_online_players->addWidget(new QLabel(CLIENT_UI_ONLINE_PLAYERS));
  connect(&m_lw_online_players, &QListWidget::itemClicked, this, &QOnlinePlayers::on_lw_online_players_item_clicked);
  vbl_online_players->addWidget(&m_lw_online_players);
  setLayout(vbl_online_players);
}

void QOnlinePlayers::update_online_client_infos() {
  m_lw_online_players.setUpdatesEnabled(false);
  client_id_t selected_id = 0;
  if (m_lw_online_players.selectedItems().size() > 0) {
    selected_id = m_lw_online_players.selectedItems()[0]->data(Qt::UserRole).toUInt();
  }
  m_lw_online_players.clear();
  for (auto &ci : m_client->m_online_client_infos) {
    if (ci.id() == m_client->m_data.m_client_id || ci.state() == CLIENT_STATE_USERNAME)
      continue;
    QListWidgetItem *item = new QListWidgetItem(ci.username().c_str());
    item->setData(Qt::UserRole, ci.id());
    m_lw_online_players.addItem(item);
    if (ci.id() == selected_id) {
      m_lw_online_players.setCurrentItem(item);
    }
  }
  m_lw_online_players.setUpdatesEnabled(true);
}

QGameArea::QGameArea(Client *client, QWidget *parent) : QClientWidget(client, parent), m_game_menu(client), m_game(client) {
  QClientVBoxLayout *vbl_game_area = new QClientVBoxLayout;
  m_sw_game.addWidget(&m_game_menu);
  m_sw_game.addWidget(&m_game);
  vbl_game_area->addWidget(&m_sw_game);
  setLayout(vbl_game_area);
}

void QGameArea::update_client_data() {
  if (m_client->m_data.m_state == CLIENT_STATE_LOBBY) {
    m_sw_game.setCurrentIndex(0);
  } else if (m_client->m_data.m_state == CLIENT_STATE_IN_GAME) {
    m_sw_game.setCurrentIndex(1);
  }
}

QInvite::QInvite(Client *client, QWidget *parent) : QClientWidget(client, parent) {
  QClientHBoxLayout *m_hbl_invite = new QClientHBoxLayout;
  m_lb_invite_text.setText(QString(CLIENT_UI_GOT_INVITED).arg(CLIENT_UI_PLAYER));
  m_hbl_invite->addWidget(&m_lb_invite_text);
  m_btn_invite_accept.setText(CLIENT_UI_ACCEPT);
  connect(&m_btn_invite_accept, &QPushButton::clicked, this, &QInvite::on_btn_invite_accept_clicked);
  m_hbl_invite->addWidget(&m_btn_invite_accept);
  m_btn_invite_decline.setText(CLIENT_UI_DECLINE);
  connect(&m_btn_invite_decline, &QPushButton::clicked, this, &QInvite::on_btn_invite_decline_clicked);
  m_hbl_invite->addWidget(&m_btn_invite_decline);
  setLayout(m_hbl_invite);
  setVisible(false);
}

void QInvite::on_btn_invite_accept_clicked() { m_client->async_invite_accept(); }

void QInvite::on_btn_invite_decline_clicked() {
  if (m_client->m_new_game_invite_queue.empty())
    return;
  m_client->m_new_game_invite_queue.pop_front();
  m_client->m_ui.update_invite();
}

void QInvite::update_invite() {
  if (m_client->m_new_game_invite_queue.empty()) {
    return;
  }
  NewGameInvite req = m_client->m_new_game_invite_queue.front();
  m_lb_invite_text.setText(QString(CLIENT_UI_GOT_INVITED).arg(req.inviter_username));
}

QGameMenu::QGameMenu(Client *client, QWidget *parent) : QClientWidget(client, parent), m_invite(client) {
  QClientVBoxLayout *vbl_game_menu = new QClientVBoxLayout;
  vbl_game_menu->addStretch(1);
  m_btn_new_game.setText(CLIENT_UI_NEW_GAME);
  m_btn_new_game.setEnabled(false);
  vbl_game_menu->addWidget(&m_btn_new_game);
  connect(&m_btn_new_game_invite, &QPushButton::clicked, this, &QGameMenu::on_btn_new_game_invite_clicked);
  m_btn_new_game_invite.setText(QString(CLIENT_UI_NEW_GAME_WITH).arg(CLIENT_UI_PLAYER));
  m_btn_new_game_invite.setEnabled(false);
  vbl_game_menu->addWidget(&m_btn_new_game_invite);
  vbl_game_menu->addWidget(&m_invite);
  vbl_game_menu->addStretch(1);
  setLayout(vbl_game_menu);
}

void QGameMenu::on_btn_new_game_invite_clicked() { m_client->async_new_game_invite(); }

void QGameMenu::update_selected_client() {
  if (m_client->m_selected_client == NULL) {
    m_btn_new_game_invite.setEnabled(false);
    m_btn_new_game_invite.setText(QString(CLIENT_UI_NEW_GAME_WITH).arg(CLIENT_UI_PLAYER));
  } else {
    m_btn_new_game_invite.setEnabled(true);
    m_btn_new_game_invite.setText(QString(CLIENT_UI_NEW_GAME_WITH).arg(m_client->m_selected_client->text()));
  }
}

void QGameMenu::update_invite() {
  m_invite.setVisible(!m_client->m_new_game_invite_queue.empty());
  m_invite.update_invite();
}

QGame::QGame(Client *client, QWidget *parent) : QClientWidget(client, parent), m_game_board(client) {
  QClientVBoxLayout *vbl_game = new QClientVBoxLayout;
  QClientHBoxLayout *hbl_game_btns = new QClientHBoxLayout;
  m_btn_game_quit.setText(CLIENT_UI_QUIT_GAME);
  connect(&m_btn_game_quit, &QPushButton::clicked, this, &QGame::on_btn_game_quit_clicked);
  hbl_game_btns->addStretch(1);
  hbl_game_btns->addWidget(&m_btn_game_quit);
  vbl_game->addLayout(hbl_game_btns);
  m_lb_game_opponent.setText(CLIENT_UI_OPPONENT);
  m_lb_game_opponent.setAlignment(Qt::AlignCenter);
  vbl_game->addWidget(&m_lb_game_opponent);
  vbl_game->addWidget(&m_game_board, 1);
  m_lb_game_player.setText(CLIENT_UI_YOU);
  m_lb_game_player.setAlignment(Qt::AlignCenter);
  vbl_game->addWidget(&m_lb_game_player);
  setLayout(vbl_game);
}

void QGame::on_btn_game_quit_clicked() {
  QMessageBox::StandardButton reply;
  reply = QMessageBox::question(this, CLIENT_UI_QUIT_GAME_MSGBOX_TITLE, CLIENT_UI_QUIT_GAME_MSGBOX_TEXT, QMessageBox::Yes | QMessageBox::No);
  if (reply == QMessageBox::Yes) {
    m_client->async_quit_game();
  }
}

void QGame::update_game() {
  m_lb_game_opponent.setText(m_client->m_game.m_opponent_username);
  m_lb_game_player.setText(CLIENT_UI_YOU);
  if (m_client->m_game.m_state == GAME_STATE_IN_GAME) {
    if (m_client->current_game_turn_is_player()) {
      QFont font = m_lb_game_player.font();
      font.setBold(true);
      m_lb_game_player.setFont(font);
      font = m_lb_game_opponent.font();
      font.setBold(false);
      m_lb_game_opponent.setFont(font);
    } else if (m_client->current_game_turn_is_opponent()) {
      QFont font = m_lb_game_player.font();
      font.setBold(false);
      m_lb_game_player.setFont(font);
      font = m_lb_game_opponent.font();
      font.setBold(true);
      m_lb_game_opponent.setFont(font);
    }
  } else if (m_client->m_game.m_state == GAME_STATE_VERDICT) {
    QFont font = m_lb_game_opponent.font();
    font.setBold(false);
    m_lb_game_opponent.setFont(font);
    font = m_lb_game_player.font();
    font.setBold(false);
    m_lb_game_player.setFont(font);
    GamePlayer winner = GAME_PLAYER_EMPTY;
    if (m_client->m_game.m_verdict == GAME_VERDICT_PLAYER1_WIN) {
      winner = GAME_PLAYER_1;
    } else if (m_client->m_game.m_verdict == GAME_VERDICT_PLAYER2_WIN) {
      winner = GAME_PLAYER_2;
    }
    if (winner != GAME_PLAYER_EMPTY) {
      client_id_t winner_id = m_client->m_game.get_player_id_from_turn(winner);
      if (winner_id == m_client->m_data.m_client_id) {
        m_lb_game_player.setText(QString(CLIENT_UI_WINNER).arg(CLIENT_UI_YOU));
      } else if (winner_id == m_client->m_game.m_opponent_id) {
        m_lb_game_opponent.setText(QString(CLIENT_UI_WINNER).arg(m_client->m_game.m_opponent_username));
      }
    }
  }
  m_game_board.update();
}

void QOnlinePlayers::on_lw_online_players_item_clicked(QListWidgetItem *item) {
  if (m_client->m_selected_client != nullptr) {
    delete m_client->m_selected_client;
    m_client->m_selected_client = nullptr;
  }
  m_client->m_selected_client = item->clone();
  m_client->m_ui.m_page_lobby.m_game_area.m_game_menu.update_selected_client();
}

QPageLobby::QPageLobby(Client *client, QWidget *parent) : QClientWidget(client, parent), m_online_players(client), m_game_area(client) {
  QClientHBoxLayout *hbl_lobby = new QClientHBoxLayout;
  hbl_lobby->addWidget(&m_online_players, 3);
  hbl_lobby->addWidget(&m_game_area, 9);
  setLayout(hbl_lobby);
}

QGameBoard::QGameBoard(Client *client, QWidget *parent) : QClientWidget(client, parent) { setCursor(Qt::PointingHandCursor); };

void QGameBoard::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setPen(Qt::NoPen);
  painter.setBrush(CLIENT_UI_GAME_BOARD_COLOR);
  painter.drawRect(rect());
  int tile_width = rect().width() / GAME_BOARD_WIDTH;
  int tile_height = rect().height() / GAME_BOARD_HEIGHT;
  for (uint8_t y = 0; y < GAME_BOARD_HEIGHT; y++) {
    for (uint8_t x = 0; x < GAME_BOARD_WIDTH; x++) {
      int radius = std::min(tile_width, tile_height) / 2 - CLIENT_UI_GAME_TILE_MARGINS;
      QPoint center(x * tile_width + tile_width / 2, y * tile_height + tile_height / 2);
      if (m_client->m_game.m_state == GAME_STATE_IN_GAME && m_client->current_game_turn_is_player()) {
        painter.setBrush(CLIENT_UI_GAME_SLOT_COLOR_ENABLE);
      } else {
        painter.setBrush(CLIENT_UI_GAME_SLOT_COLOR_DISABLE);
      }
      GamePlayer cell = m_client->m_game.m_board[y][x];
      client_id_t cell_id = m_client->m_game.get_player_id_from_turn(cell);
      if (m_client->m_game.m_state == GAME_STATE_VERDICT && m_client->m_game.m_connect_mask[y][x]) {
        painter.setPen(QPen(CLIENT_UI_GAME_CONNECT_TOKEN_BORDER_COLOR, CLIENT_UI_GAME_CONNECT_TOKEN_BORDER_WIDTH));
      } else {
        painter.setPen(Qt::NoPen);
      }
      if (cell_id == m_client->m_data.m_client_id) {
        painter.setBrush(CLIENT_UI_GAME_TOKEN_COLOR_PLAYER);
      } else if (cell_id == m_client->m_game.m_opponent_id) {
        painter.setBrush(CLIENT_UI_GAME_TOKEN_COLOR_OPPONENT);
      }
      painter.drawEllipse(center, radius, radius);
    }
  }
}
void QGameBoard::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    int tile_width = rect().width() / GAME_BOARD_WIDTH;
    QPoint pos = event->pos();
    int col = pos.x() / tile_width;
    col = std::clamp(col, 0, GAME_BOARD_WIDTH - 1);
    spdlog::debug("Player attempt to click on column [{}]", col);
    if (m_client->m_game.m_state == GAME_STATE_IN_GAME && m_client->current_game_turn_is_player()) {
      m_client->m_message_handler.send_command_player_make_move(col);
    }
  }
}

QClientUI::QClientUI(Client *client, QWidget *parent) : QClientWidget(client, parent), m_page_username(client), m_page_lobby(client) {
  m_sw_pages.addWidget(&m_page_username);
  m_sw_pages.addWidget(&m_page_lobby);
  QClientVBoxLayout *vbl_main = new QClientVBoxLayout(CLIENT_UI_WINDOW_MARGIN);
  vbl_main->addWidget(&m_sw_pages);
  setLayout(vbl_main);
  setWindowTitle(CLIENT_UI_WINDOW_TITLE);
  resize(640, 480);
};

void QClientUI::username_check_failed() {
  QMetaObject::invokeMethod(
      this, [this]() { QMessageBox::information(this, CLIENT_UI_USERNAME_CHECK_FAILED_MSGBOX_TITLE, CLIENT_UI_USERNAME_CHECK_FAILED_MSGBOX_TEXT); });
}

void QClientUI::update_server_connection() {
  QMetaObject::invokeMethod(this, [this]() { m_page_username.update_server_connection(); });
}

void QClientUI::update_client_data() {
  QMetaObject::invokeMethod(this, [this]() {
    if (m_client->m_data.m_state == CLIENT_STATE_USERNAME) {
      m_sw_pages.setCurrentIndex(0);
      setWindowTitle(CLIENT_UI_WINDOW_TITLE);
    } else if (m_client->m_data.m_state == CLIENT_STATE_LOBBY) {
      m_sw_pages.setCurrentIndex(1);
      setWindowTitle(QString(CLIENT_UI_WINDOW_TITLE_WITH_USERNAME).arg(m_client->m_data.m_username));
    }
    m_page_lobby.m_game_area.update_client_data();
  });
}

void QClientUI::update_online_client_infos() {
  QMetaObject::invokeMethod(this, [this]() { m_page_lobby.m_online_players.update_online_client_infos(); });
}

void QClientUI::update_game() {
  QMetaObject::invokeMethod(this, [this]() { m_page_lobby.m_game_area.m_game.update_game(); });
}

void QClientUI::update_invite() {
  QMetaObject::invokeMethod(this, [this]() { m_page_lobby.m_game_area.m_game_menu.update_invite(); });
}

} // namespace C4