#include "client.hpp"
#include "message.pb.h"
#include <cstring>
#include <netinet/in.h>
#include <spdlog/spdlog.h>

using namespace C4;

Client::Client(uv_loop_t *loop, const char *host, int port) : m_msg_hdl(this) {
  m_loop = loop;
  m_host = (char *)host;
  m_port = port;
  uv_tcp_init(m_loop, &m_client);
  m_client.data = this;
  std::memset(&m_server_addr, 0, sizeof(struct sockaddr_in));
  m_client_id = 0;
  m_total_clients = 0;
  m_state = CLIENT_STATE_LOBBY;
}

Client::~Client() {}

void Client::run() {
  uv_ip4_addr(m_host, m_port, &m_server_addr);
  uv_connect_t *connect = new uv_connect_t;
  connect->data = this;
  uv_tcp_connect(connect, &m_client, (struct sockaddr *)&m_server_addr,
                 Client::on_connect);
}

void Client::on_connect(uv_connect_t *connect, int status) {
  Client *self = (Client *)connect->data;

  if (status != 0) {
    // something error
  }

  spdlog::info("Connected to server");

  // Start read
  uv_read_start((uv_stream_t *)&self->m_client, Client::on_alloc,
                Client::on_read);
}

void Client::on_alloc(uv_handle_t *handle, unsigned long size, uv_buf_t *buf) {
  buf->base = (char *)malloc(size);
  buf->len = size;
}

void Client::on_read(uv_stream_t *stream, long nread, const uv_buf_t *buf) {
  Client *self = (Client *)stream->data;
  if (nread < 0) {
    // Server close
    uv_close((uv_handle_t *)stream, Client::on_close);
    free(buf->base);
    return;
  }

  if (self->m_ring_buf.write(buf->base, nread) < 0) {
    free(buf->base);
    return;
  }
  constexpr uint32_t MAX_MSG_SIZE = 16 * 1024 * 1024;
  while (self->m_ring_buf.m_size >= 4) {
    uint32_t msg_size = 0;
    self->m_ring_buf.peek(&msg_size, sizeof(uint32_t), 4);
    msg_size = ntohl(msg_size);
    spdlog::info("msg size: {}", msg_size);
    if (msg_size > MAX_MSG_SIZE)
      break;
    if (self->m_ring_buf.m_size < 4 + msg_size)
      break;
    self->m_ring_buf.consume(4);
    Message msg;
    msg.ParseFromArray(self->m_ring_buf.get_read_ptr(), msg_size);
    self->m_msg_hdl.handle_message(&msg);
    self->m_ring_buf.consume(msg_size);
  }

  free(buf->base);
}

void Client::on_write(uv_write_t *write, int status) {
  if (status != 0) {
    // something error
  }
  delete[] write->bufs;
  delete write;
}

void Client::on_close(uv_handle_t *handle) { printf("Server closed\n"); }