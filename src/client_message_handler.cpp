#include "client.hpp"

using namespace C4;

Client::MessageHandler::MessageHandler(Client *client) { m_client = client; }
Client::MessageHandler::~MessageHandler() {}

void Client::MessageHandler::handle_message(Message *message) {
  switch (message->payload_type_case()) {
  case Message::kEventPayload:
    handle_event(&message->event_payload());
    break;
  // Client will only receive events
  case Message::PAYLOAD_TYPE_NOT_SET:
  default:
    break;
  }
}

void Client::MessageHandler::handle_event(const EventPayload *event_payload) {
  switch (event_payload->payload_case()) {
  case EventPayload::kEventServerConnected:
    handle_event_server_connected(&event_payload->event_server_connected());
    break;
  case EventPayload::kEventLobbyUpdated:
    handle_event_lobby_updated(&event_payload->event_lobby_updated());
    break;
  default:
    break;
  }
}

void Client::MessageHandler::handle_event_server_connected(
    const EventServerConnected *sc) {
  std::lock_guard lock(m_client->m_lock);
  m_client->m_client_id = sc->client_id();
  spdlog::info("Set client id: {}", m_client->m_client_id);
}

void Client::MessageHandler::handle_event_lobby_updated(
    const EventLobbyUpdated *lu) {
  std::lock_guard lock(m_client->m_lock);
  m_client->m_total_clients = lu->total_clients();
  spdlog::info("Set total clients: {}", m_client->m_total_clients);
}

void Client::MessageHandler::send_message(Message *msg) {
  WriteRequest *wr = new WriteRequest(msg);
  wr->req.data = this;
  uv_write((uv_write_t *)wr, (uv_stream_t *)&m_client->m_session, &wr->buf, 1,
           Client::on_write);
}

void Client::MessageHandler::send_command_enter_lobby() {
  Message msg;
  CommandPayload *cp = msg.mutable_command_payload();
  CommandEnterLobby *el = cp->mutable_command_enter_lobby();
  el->set_username(m_client->m_username);
  spdlog::info("Entering lobby with username: {}", m_client->m_username);
  send_message(&msg);
}