#include "client.hpp"
#include "qboxlayout.h"
#include "qobjectdefs.h"

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
  connect(m_le_username, &QLineEdit::textChanged, this,
          &QUI::on_le_username_text_changed);
  fl_username->addRow(new QLabel(tr("Username")), m_le_username);
  vbl_username->addLayout(fl_username);
  m_lb_server_status = new QLabel(tr("Server disconnected"));
  vbl_username->addWidget(m_lb_server_status);
  QHBoxLayout *hbl_btn = new QHBoxLayout;
  hbl_btn->setContentsMargins(0, 0, 0, 0);
  m_btn_enter_lobby = new QPushButton(tr("Enter lobby"));
  hbl_btn->addWidget(m_btn_enter_lobby);
  hbl_btn->addStretch(1);
  vbl_username->addLayout(hbl_btn);
  vbl_username->addStretch(1);
  connect(m_btn_enter_lobby, &QPushButton::clicked, this,
          &QUI::on_btn_enter_lobby_clicked);
  wg_page_username->setLayout(vbl_username);

  QWidget *wg_page_lobby = new QWidget;
  QHBoxLayout *hbl_lobby = new QHBoxLayout;
  hbl_lobby->setContentsMargins(0, 0, 0, 0);

  QVBoxLayout *vbl_online_players = new QVBoxLayout;
  vbl_online_players->setContentsMargins(0, 0, 0, 0);
  vbl_online_players->addWidget(new QLabel(tr("Online Players:")));
  m_lw_online_players = new QListWidget;
  vbl_online_players->addWidget(m_lw_online_players);

  QVBoxLayout *vbl_game = new QVBoxLayout;

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
  m_client->m_is_connected = is_connected;
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
  m_client->m_state = CLIENT_STATE_LOBBY;
  QMetaObject::invokeMethod(this, [this]() { m_sw_pages->setCurrentIndex(1); });
}

void Client::QUI::username_check_failed() {
  QMetaObject::invokeMethod(this, [this]() {
    QMessageBox::information(
        this, tr("Cannot use this username"),
        tr("This username is already taken, please try another."));
  });
}

void Client::QUI::on_btn_enter_lobby_clicked() {
  m_client->enter_lobby_async();
}

void Client::QUI::on_le_username_text_changed(const QString &text) {
  strncpy(m_client->m_username, text.toUtf8().constData(), USERNAME_MAX_SIZE);
}

} // namespace C4