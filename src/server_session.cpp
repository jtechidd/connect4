#include "server.hpp"

using namespace C4;

Server::Session::Session(uv_loop_t *loop, Server *server, session_id_t id) {
  m_loop = loop;
  m_server = server;
  m_id = id;
  m_curr_game_id = 0;
  uv_tcp_init(m_loop, &m_session);
  m_session.data = this;
}

Server::Session::~Session() {}

void Server::Session::run() {
  uv_read_start((uv_stream_t *)&m_session, on_alloc, Server::Session::on_read);
  m_server->m_msg_hdl.emit_event_server_connected(m_id);
}

void Server::Session::on_alloc(uv_handle_t *handle, unsigned long size,
                               uv_buf_t *buf) {
  buf->base = new char[size];
  buf->len = size;
}

void Server::Session::on_read(uv_stream_t *stream, long nread,
                              const uv_buf_t *buf) {
  Session *self = (Session *)stream->data;
  uint32_t msg_size = 0;
  Message msg;

  if (nread < 0) {
    uv_close((uv_handle_t *)stream, Server::Session::on_close);
    goto cleanup;
  }

  while (self->m_ring_buf.m_size >= MSG_SIZE_NBYTES) {
    self->m_ring_buf.peek(&msg_size, sizeof(uint32_t), MSG_SIZE_NBYTES);
    msg_size = ntohl(msg_size);
    if (msg_size > MSG_MAX_SIZE)
      break;
    if (self->m_ring_buf.m_size < MSG_SIZE_NBYTES + msg_size)
      break;
    self->m_ring_buf.consume(MSG_SIZE_NBYTES);
    msg.ParseFromArray(self->m_ring_buf.get_read_ptr(), msg_size);
    self->m_server->m_msg_hdl.handle_message(&msg);
    self->m_ring_buf.consume(msg_size);
  }

cleanup:
  free(buf->base);
}

void Server::Session::on_write(uv_write_t *write, int status) {
  if (status != 0) {
  }
  free(write);
}

void Server::Session::on_close(uv_handle_t *handle) {
  Session *self = (Session *)handle->data;
  self->m_server->disconnect(self->m_id);
}
