#include "server.hpp"

namespace C4 {

Server::ClientConnection::ClientConnection(uv_loop_t *loop, Server *server, client_id_t id) {
  m_uv_loop = loop;
  m_server = server;
  m_id = id;
  m_current_game_id = 0;
  uv_tcp_init(m_uv_loop, &m_uv_tcp_connection);
  m_uv_tcp_connection.data = this;
  m_state = CLIENT_STATE_USERNAME;
  memset(m_username, 0, sizeof(m_username));
}

Server::ClientConnection::~ClientConnection() {}

void Server::ClientConnection::run() {
  uv_read_start((uv_stream_t *)&m_uv_tcp_connection, on_uv_tcp_connection_alloc, Server::ClientConnection::on_uv_tcp_connection_read);
  m_server->m_message_handler.emit_event_server_connected(m_id);
}

void Server::ClientConnection::on_uv_tcp_connection_alloc(uv_handle_t *handle, unsigned long size, uv_buf_t *buf) {
  buf->base = new char[size];
  buf->len = size;
}

void Server::ClientConnection::on_uv_tcp_connection_read(uv_stream_t *stream, long nread, const uv_buf_t *buf) {
  ClientConnection *self = (ClientConnection *)stream->data;
  uint32_t msg_size = 0;
  Message msg;

  if (nread < 0) {
    uv_close((uv_handle_t *)stream, Server::ClientConnection::on_uv_tcp_connection_close);
    goto cleanup;
  }

  if (self->m_ring_buffer.write(buf->base, nread) < 0) {
    goto cleanup;
  }

  while (self->m_ring_buffer.m_size >= MSG_SIZE_NBYTES) {
    self->m_ring_buffer.peek(&msg_size, sizeof(uint32_t), MSG_SIZE_NBYTES);
    msg_size = ntohl(msg_size);
    if (msg_size > MSG_MAX_SIZE)
      break;
    if (self->m_ring_buffer.m_size < MSG_SIZE_NBYTES + msg_size)
      break;
    self->m_ring_buffer.consume(MSG_SIZE_NBYTES);
    uint8_t *raw_msg = new uint8_t[msg_size];
    self->m_ring_buffer.read(raw_msg, msg_size, msg_size);
    msg.ParseFromArray(raw_msg, msg_size);
    delete[] raw_msg;
    self->m_server->m_message_handler.handle_message(self->m_id, &msg);
  }

cleanup:
  delete[] buf->base;
}

void Server::ClientConnection::on_uv_tcp_connection_write(uv_write_t *write, int status) {
  if (status != 0) {
  }
  delete write;
}

void Server::ClientConnection::on_uv_tcp_connection_close(uv_handle_t *handle) {
  ClientConnection *self = (ClientConnection *)handle->data;
  self->m_server->disconnect(self->m_id);
}

}; // namespace C4