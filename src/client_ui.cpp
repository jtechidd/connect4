#include "client_ui.hpp"
#include "client.hpp"

namespace C4 {

ClientUI::QGame::QGame(QWidget *parent) : QWidget(parent) {};
void ClientUI::QGame::paintEvent(QPaintEvent *) {}

ClientUI::ClientUI(Client *client, QWidget *parent) : QWidget(parent) {
  m_client = client;

  m_sw_pages = new QStackedWidget;

  QWidget *wg_page_username = new QWidget();
  QVBoxLayout *vbl_username = new QVBoxLayout;
  vbl_username->setContentsMargins(0, 0, 0, 0);
  QFormLayout *fl_username = new QFormLayout;
  fl_username->setContentsMargins(0, 0, 0, 0);
  m_le_username = new QLineEdit;
  m_le_username->setMaxLength(USERNAME_MAX_SIZE);
  connect(m_le_username, &QLineEdit::textChanged, this, &ClientUI::on_le_username_text_changed);
  fl_username->addRow(new QLabel(tr("Username")), m_le_username);
  vbl_username->addLayout(fl_username);
  m_lb_server_status = new QLabel(tr("Server disconnected"));
  vbl_username->addWidget(m_lb_server_status);
  QHBoxLayout *hbl_btn = new QHBoxLayout;
  hbl_btn->setContentsMargins(0, 0, 0, 0);
  m_btn_enter_lobby = new QPushButton(tr("Enter lobby"));
  m_btn_enter_lobby->setEnabled(false);
  hbl_btn->addWidget(m_btn_enter_lobby);
  hbl_btn->addStretch(1);
  vbl_username->addLayout(hbl_btn);
  vbl_username->addStretch(1);
  connect(m_btn_enter_lobby, &QPushButton::clicked, this, &ClientUI::on_btn_enter_lobby_clicked);
  wg_page_username->setLayout(vbl_username);

  QWidget *wg_page_lobby = new QWidget;
  QHBoxLayout *hbl_lobby = new QHBoxLayout;
  hbl_lobby->setContentsMargins(0, 0, 0, 0);

  QVBoxLayout *vbl_online_players = new QVBoxLayout;
  vbl_online_players->setContentsMargins(0, 0, 0, 0);
  vbl_online_players->addWidget(new QLabel(tr("Online Players:")));
  m_lw_online_players = new QListWidget;
  connect(m_lw_online_players, &QListWidget::itemClicked, this, &ClientUI::on_lw_online_players_item_clicked);
  vbl_online_players->addWidget(m_lw_online_players);

  m_sw_game = new QStackedWidget;

  QWidget *wg_game_menu = new QWidget;
  QVBoxLayout *vbl_game_menu = new QVBoxLayout;
  vbl_game_menu->setContentsMargins(0, 0, 0, 0);
  QPushButton *btn_new_game = new QPushButton(tr("New Game"));
  btn_new_game->setEnabled(false);
  m_btn_new_game_invite = new QPushButton("New Game With Player");
  m_btn_new_game_invite->setEnabled(false);
  connect(m_btn_new_game_invite, &QPushButton::clicked, this, &ClientUI::on_btn_new_game_invite_clicked);
  vbl_game_menu->addStretch(1);
  vbl_game_menu->addWidget(btn_new_game);
  vbl_game_menu->addWidget(m_btn_new_game_invite);

  m_wg_invite = new QWidget;
  QHBoxLayout *m_hbl_invite = new QHBoxLayout;
  m_hbl_invite->setContentsMargins(0, 0, 0, 0);
  m_lb_invite_text = new QLabel(tr("You got invited from player"));
  m_btn_invite_accept = new QPushButton(tr("Accept"));
  connect(m_btn_invite_accept, &QPushButton::clicked, this, &ClientUI::on_btn_invite_accept_clicked);
  m_btn_invite_decline = new QPushButton(tr("Decline"));
  connect(m_btn_invite_decline, &QPushButton::clicked, this, &ClientUI::on_btn_invite_decline_clicked);
  m_hbl_invite->addWidget(m_lb_invite_text);
  m_hbl_invite->addWidget(m_btn_invite_accept);
  m_hbl_invite->addWidget(m_btn_invite_decline);
  m_wg_invite->setLayout(m_hbl_invite);
  m_wg_invite->setVisible(false);

  vbl_game_menu->addWidget(m_wg_invite);
  vbl_game_menu->addStretch(1);
  wg_game_menu->setLayout(vbl_game_menu);

  QWidget *wg_game = new QWidget;
  QVBoxLayout *vbl_game = new QVBoxLayout;
  wg_game->setLayout(vbl_game);

  m_sw_game->addWidget(wg_game_menu);
  m_sw_game->addWidget(wg_game);

  hbl_lobby->addLayout(vbl_online_players, 3);
  hbl_lobby->addWidget(m_sw_game, 9);
  wg_page_lobby->setLayout(hbl_lobby);

  m_sw_pages->addWidget(wg_page_username);
  m_sw_pages->addWidget(wg_page_lobby);

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->addWidget(m_sw_pages);

  setWindowTitle(tr("Connect4"));
};

void ClientUI::update_server_connection(bool is_connected) {
  QMetaObject::invokeMethod(this, [this, is_connected]() {
    if (is_connected) {
      m_lb_server_status->setText(tr("Server connected"));
      m_btn_enter_lobby->setEnabled(true);
    } else {
      m_sw_pages->setCurrentIndex(0);
      m_lb_server_status->setText(tr("Server disconnected"));
      m_btn_enter_lobby->setEnabled(false);
      setWindowTitle("Connect4");
    }
  });
}

void ClientUI::lobby_entered() {
  QMetaObject::invokeMethod(this, [this]() {
    m_sw_pages->setCurrentIndex(1);
    setWindowTitle(QString("Connect4 (%1)").arg(m_client->m_username));
  });
}

void ClientUI::lobby_updated(const EventLobbyUpdated *lu) {
  QMetaObject::invokeMethod(this, [this, lu = *lu]() {
    m_lw_online_players->setUpdatesEnabled(false);
    client_id_t selected_id = 0;
    if (m_lw_online_players->selectedItems().size() > 0) {
      selected_id = m_lw_online_players->selectedItems()[0]->data(Qt::UserRole).toUInt();
    }
    m_lw_online_players->clear();
    for (auto &ci : lu.clients()) {
      if (ci.id() == m_client->m_client_id || ci.state() == CLIENT_STATE_USERNAME)
        continue;
      QListWidgetItem *item = new QListWidgetItem(ci.username().c_str());
      item->setData(Qt::UserRole, ci.id());
      m_lw_online_players->addItem(item);
      if (ci.id() == selected_id) {
        m_lw_online_players->setCurrentItem(item);
      }
    }
    m_lw_online_players->setUpdatesEnabled(true);
  });
}

void ClientUI::username_check_failed() {
  QMetaObject::invokeMethod(
      this, [this]() { QMessageBox::information(this, tr("Cannot use this username"), tr("This username is already taken, please try another.")); });
}

void ClientUI::new_game_joined(const EventNewGameJoined *payload) {
  QMetaObject::invokeMethod(this, [this]() { m_sw_game->setCurrentIndex(1); });
}

void ClientUI::update_invite() {
  QMetaObject::invokeMethod(this, [this]() {
    if (m_client->m_new_game_invite_queue.empty()) {
      m_wg_invite->setVisible(false);
      return;
    }
    NewGameInvite req = m_client->m_new_game_invite_queue.front();
    m_lb_invite_text->setText(QString("You got invited from %1").arg(req.inviter_username));
    m_wg_invite->setVisible(true);
  });
}

void ClientUI::on_btn_enter_lobby_clicked() { m_client->async_enter_lobby(); }

void ClientUI::on_le_username_text_changed(const QString &text) {
  snprintf(m_client->m_username, sizeof(m_client->m_username), "%s", text.toUtf8().constData());
}

void ClientUI::on_lw_online_players_item_clicked(QListWidgetItem *item) {
  m_client->m_selected_client_id = item->data(Qt::UserRole).toUInt();
  m_btn_new_game_invite->setEnabled(true);
  m_btn_new_game_invite->setText(QString("New Game With %1").arg(item->text()));
}

void ClientUI::on_btn_new_game_invite_clicked() { m_client->async_new_game_invite(); }

void ClientUI::on_btn_invite_accept_clicked() { m_client->async_invite_accept(); }

void ClientUI::on_btn_invite_decline_clicked() {
  if (m_client->m_new_game_invite_queue.empty())
    return;
  m_client->m_new_game_invite_queue.pop_front();
  m_client->m_ui.update_invite();
}

} // namespace C4