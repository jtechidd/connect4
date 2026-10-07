#include "client.hpp"
#include "common.hpp"

namespace C4 {

const char *CLIENT_DEFAULT_HOST = "localhost";
const int CLIENT_DEFAULT_PORT = 8080;

Client::Client(int argc, char **argv, const char *host, int port)
    : m_message_handler(this), m_qt_app(argc, argv), m_qt_ui(this) {
  m_uv_loop = g_uv_loop;
  m_host = (char *)host;
  m_port = port;
  m_client_id = 0;
  m_total_clients = 0;
  m_state = CLIENT_STATE_USERNAME;
  m_is_connected = false;
  memset(m_username, 0, sizeof(m_username));

  uv_tcp_init(m_uv_loop, &m_uv_tcp_connection);
  m_uv_tcp_connection.data = this;
  uv_async_init(m_uv_loop, &m_uv_async_keep_alive, NULL);
  m_uv_async_keep_alive.data = this;
  uv_async_init(m_uv_loop, &m_uv_async_enter_lobby,
                Client::on_uv_async_enter_lobby_awake);
  m_uv_async_enter_lobby.data = this;
  uv_async_init(m_uv_loop, &m_uv_async_stop_loop,
                Client::on_uv_async_stop_uv_loop_awake);
  m_uv_async_stop_loop.data = this;
  uv_timer_init(m_uv_loop, &m_uv_timer_try_connect);
  m_uv_timer_try_connect.data = this;
}

Client::~Client() {}

void Client::run_uv_loop() {
  uv_timer_start(&m_uv_timer_try_connect,
                 Client::on_uv_timer_try_connect_timeout, 1000, 1000);
  uv_run(m_uv_loop, UV_RUN_DEFAULT);
}

int Client::run_qt_ui() {
  m_qt_ui.show();
  return m_qt_app.exec();
}

void Client::run() {
  m_thread_uv_loop = std::thread(&Client::run_uv_loop, this);
  run_qt_ui();
  spdlog::info("UI stopped");
  async_stop_uv_loop();
  m_thread_uv_loop.join();
}

void Client::on_uv_tcp_server_connect(uv_connect_t *connect, int status) {
  Client *self = (Client *)connect->data;
  if (status != 0) {
    self->m_qt_ui.update_server_connection(false);
    goto cleanup;
  }

  spdlog::info("Connected to server");
  uv_read_start((uv_stream_t *)&self->m_uv_tcp_connection,
                Client::on_uv_tcp_connection_alloc,
                Client::on_uv_tcp_connection_read);
  self->m_qt_ui.update_server_connection(true);
cleanup:
  delete connect;
}

void Client::on_uv_tcp_connection_alloc(uv_handle_t *handle, unsigned long size,
                                        uv_buf_t *buf) {
  buf->base = new char[size];
  buf->len = size;
}

void Client::on_uv_tcp_connection_read(uv_stream_t *stream, long nread,
                                       const uv_buf_t *buf) {
  Client *self = (Client *)stream->data;
  uint32_t msg_size = 0;
  Message msg;

  if (nread < 0) {
    uv_close((uv_handle_t *)stream, Client::on_uv_tcp_connection_close);
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
    self->m_message_handler.handle_message(&msg);
  }
cleanup:
  delete[] buf->base;
}

void Client::on_uv_tcp_connection_write(uv_write_t *write, int status) {
  if (status != 0) {
    // TODO: handle
  }
  delete write;
}

void Client::on_uv_tcp_connection_close(uv_handle_t *handle) {
  Client *self = (Client *)handle->data;
  spdlog::info("Disconnencted from server");
  self->m_qt_ui.update_server_connection(false);
  self->m_state = CLIENT_STATE_USERNAME;
  uv_timer_start(&self->m_uv_timer_try_connect,
                 Client::on_uv_timer_try_connect_timeout, 1000, 1000);
}

void Client::on_uv_async_stop_uv_loop_awake(uv_async_t *handle) {
  Client *self = (Client *)handle->data;
  uv_stop(self->m_uv_loop);
}

void Client::on_uv_async_enter_lobby_awake(uv_async_t *handle) {
  Client *self = (Client *)handle->data;
  self->m_message_handler.send_command_enter_lobby();
}

void Client::async_stop_uv_loop() { uv_async_send(&m_uv_async_stop_loop); }

void Client::async_enter_lobby() { uv_async_send(&m_uv_async_enter_lobby); }

void Client::on_uv_timer_try_connect_timeout(uv_timer_t *timer) {
  Client *self = (Client *)timer->data;
  if (self->m_is_connected) {
    uv_timer_stop(timer);
    return;
  }
  spdlog::info("Connecting to server...");
  uv_tcp_init(self->m_uv_loop, &self->m_uv_tcp_connection);
  self->m_uv_tcp_connection.data = self;
  uv_ip4_addr(self->m_host, self->m_port, &self->m_server_address);
  uv_connect_t *connect = new uv_connect_t;
  connect->data = self;
  uv_tcp_connect(connect, &self->m_uv_tcp_connection,
                 (struct sockaddr *)&self->m_server_address,
                 Client::on_uv_tcp_server_connect);
}

}; // namespace C4