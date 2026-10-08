#include "QtWidgets"
#include "client.hpp"
#include "common.hpp"
#include "message.pb.h"
#include "qlabel.h"
#include "qlistwidget.h"
#include "qnamespace.h"
#include "qobject.h"
#include "qpushbutton.h"

namespace C4 {
Client::QUI::QUI(Client *client, QWidget *parent) : QWidget(parent) {
  m_client = client;

  m_sw_pages = new QStackedWidget;

  QWidget *wg_page_username = new QWidget();
  QVBoxLayout *vbl_username = new QVBoxLayout;
  vbl_username->setContentsMargins(0, 0, 0, 0);
  QFormLayout *fl_username = new QFormLayout;
  fl_username->setContentsMargins(0, 0, 0, 0);
  m_le_username = new QLineEdit;
  m_le_username->setMaxLength(USERNAME_MAX_SIZE);
  connect(m_le_username, &QLineEdit::textChanged, this, &QUI::on_le_username_text_changed);
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
  connect(m_btn_enter_lobby, &QPushButton::clicked, this, &QUI::on_btn_enter_lobby_clicked);
  wg_page_username->setLayout(vbl_username);

  QWidget *wg_page_lobby = new QWidget;
  QHBoxLayout *hbl_lobby = new QHBoxLayout;
  hbl_lobby->setContentsMargins(0, 0, 0, 0);

  QVBoxLayout *vbl_online_players = new QVBoxLayout;
  vbl_online_players->setContentsMargins(0, 0, 0, 0);
  vbl_online_players->addWidget(new QLabel(tr("Online Players:")));
  m_lw_online_players = new QListWidget;
  connect(m_lw_online_players, &QListWidget::itemClicked, this, &QUI::on_lw_online_players_item_clicked);
  vbl_online_players->addWidget(m_lw_online_players);

  QVBoxLayout *vbl_game = new QVBoxLayout;
  QPushButton *btn_new_game = new QPushButton(tr("New Game"));
  btn_new_game->setEnabled(false);
  m_btn_new_game_invite = new QPushButton("New Game With player");
  m_btn_new_game_invite->setEnabled(false);
  connect(m_btn_new_game_invite, &QPushButton::clicked, this, &QUI::on_btn_new_game_invite_clicked);
  vbl_game->addStretch(1);
  vbl_game->addWidget(btn_new_game);
  vbl_game->addWidget(m_btn_new_game_invite);

  m_wg_invite = new QWidget;
  QHBoxLayout *m_hbl_invite = new QHBoxLayout;
  m_hbl_invite->setContentsMargins(0, 0, 0, 0);
  m_lb_invite_text = new QLabel(tr("You got invited from player"));
  m_btn_invite_accept = new QPushButton(tr("Accept"));
  connect(m_btn_invite_accept, &QPushButton::clicked, this, &QUI::on_btn_invite_accept_clicked);
  m_btn_invite_decline = new QPushButton(tr("Decline"));
  connect(m_btn_invite_decline, &QPushButton::clicked, this, &QUI::on_btn_invite_decline_clicked);
  m_hbl_invite->addWidget(m_lb_invite_text);
  m_hbl_invite->addWidget(m_btn_invite_accept);
  m_hbl_invite->addWidget(m_btn_invite_decline);
  m_wg_invite->setLayout(m_hbl_invite);
  m_wg_invite->setVisible(false);

  vbl_game->addWidget(m_wg_invite);
  vbl_game->addStretch(1);

  hbl_lobby->addLayout(vbl_online_players, 3);
  hbl_lobby->addLayout(vbl_game, 9);
  wg_page_lobby->setLayout(hbl_lobby);

  m_sw_pages->addWidget(wg_page_username);
  m_sw_pages->addWidget(wg_page_lobby);

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->addWidget(m_sw_pages);

  setWindowTitle(tr("Connect4"));
};

void Client::QUI::update_server_connection(bool is_connected) {
  QMetaObject::invokeMethod(this, [this, is_connected]() {
    if (is_connected) {
      m_lb_server_status->setText(tr("Server connected"));
      m_btn_enter_lobby->setEnabled(true);
    } else {
      m_sw_pages->setCurrentIndex(0);
      m_lb_server_status->setText(tr("Server disconnected"));
      m_btn_enter_lobby->setEnabled(false);
    }
  });
}

void Client::QUI::lobby_entered() {
  QMetaObject::invokeMethod(this, [this]() { m_sw_pages->setCurrentIndex(1); });
}

void Client::QUI::lobby_updated(const EventLobbyUpdated *lu) {
  QMetaObject::invokeMethod(this, [this, lu = *lu]() {
    m_lw_online_players->setUpdatesEnabled(false);
    client_id_t selected_id = 0;
    if (m_lw_online_players->selectedItems().size() > 0) {
      selected_id = m_lw_online_players->selectedItems()[0]->data(Qt::UserRole).toUInt();
      spdlog::debug("Previous selected id: {}", selected_id);
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

void Client::QUI::username_check_failed() {
  QMetaObject::invokeMethod(
      this, [this]() { QMessageBox::information(this, tr("Cannot use this username"), tr("This username is already taken, please try another.")); });
}

void Client::QUI::update_invite() {
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

void Client::QUI::on_btn_enter_lobby_clicked() { m_client->async_enter_lobby(); }

void Client::QUI::on_le_username_text_changed(const QString &text) { strncpy(m_client->m_username, text.toUtf8().constData(), USERNAME_MAX_SIZE); }

void Client::QUI::on_lw_online_players_item_clicked(QListWidgetItem *item) {
  m_client->m_selected_client_id = item->data(Qt::UserRole).toUInt();
  m_btn_new_game_invite->setEnabled(true);
  m_lb_invite_text->setText(QString("New Game With %1").arg(item->text()));
}

void Client::QUI::on_btn_new_game_invite_clicked() { m_client->async_new_game_invite(); }

void Client::QUI::on_btn_invite_accept_clicked() { m_client->async_invite_accept(); }

void Client::QUI::on_btn_invite_decline_clicked() {
  if (m_client->m_new_game_invite_queue.empty())
    return;
  m_client->m_new_game_invite_queue.pop_front();
  m_client->m_qt_ui.update_invite();
}

} // namespace C4