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
  std::lock_guard<std::mutex> lock(m_client->m_lock);
  m_client->m_client_id = sc->client_id();
  spdlog::info("Set client id: {}", m_client->m_client_id);
}

void Client::MessageHandler::handle_event_lobby_updated(
    const EventLobbyUpdated *lu) {
  std::lock_guard<std::mutex> lock(m_client->m_lock);
  m_client->m_total_clients = lu->total_clients();
  spdlog::info("Set total clients: {}", m_client->m_total_clients);
}
