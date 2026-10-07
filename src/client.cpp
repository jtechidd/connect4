#include "client.hpp"
#include "common.hpp"

namespace C4 {

const char *CLIENT_DEFAULT_HOST = "localhost";
const int CLIENT_DEFAULT_PORT = 8080;

Client::Client(int argc, char **argv, const char *host, int port)
    : m_msg_hdl(this), m_qapp(argc, argv), m_qui(this) {
  m_loop = g_loop;
  m_host = (char *)host;
  m_port = port;
  m_client_id = 0;
  m_total_clients = 0;
  m_state = CLIENT_STATE_USERNAME;
  m_is_connected = false;
  memset(m_username, 0, sizeof(m_username));

  uv_tcp_init(m_loop, &m_connection);
  m_connection.data = this;
  uv_async_init(m_loop, &m_keep_alive, NULL);
  m_keep_alive.data = this;
  uv_async_init(m_loop, &m_enter_lobby, Client::enter_lobby_async_cb);
  m_enter_lobby.data = this;
  uv_async_init(m_loop, &m_stop, Client::stop_async_cb);
  m_stop.data = this;
  uv_timer_init(m_loop, &m_try_connect);
  m_try_connect.data = this;
}

Client::~Client() {}

void Client::run_uv() {
  uv_timer_start(&m_try_connect, Client::on_try_connect, 1000, 1000);
  uv_run(m_loop, UV_RUN_DEFAULT);
}

int Client::run_ui() {
  m_qui.show();
  return m_qapp.exec();
}

void Client::run() {
  m_thread_uv = std::thread(&Client::run_uv, this);
  run_ui();
  spdlog::info("UI stopped");
  stop_async();
  m_thread_uv.join();
}

void Client::on_connect(uv_connect_t *connect, int status) {
  Client *self = (Client *)connect->data;
  if (status != 0) {
    self->m_qui.update_server_connection(false);
    goto cleanup;
  }

  spdlog::info("Connected to server");
  uv_read_start((uv_stream_t *)&self->m_connection, Client::on_alloc,
                Client::on_read);
  self->m_qui.update_server_connection(true);
cleanup:
  delete connect;
}

void Client::on_alloc(uv_handle_t *handle, unsigned long size, uv_buf_t *buf) {
  buf->base = new char[size];
  buf->len = size;
}

void Client::on_read(uv_stream_t *stream, long nread, const uv_buf_t *buf) {
  Client *self = (Client *)stream->data;
  uint32_t msg_size = 0;
  Message msg;

  if (nread < 0) {
    uv_close((uv_handle_t *)stream, Client::on_close);
    goto cleanup;
  }

  if (self->m_ring_buf.write(buf->base, nread) < 0) {
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
    uint8_t *raw_msg = new uint8_t[msg_size];
    self->m_ring_buf.read(raw_msg, msg_size, msg_size);
    msg.ParseFromArray(raw_msg, msg_size);
    delete[] raw_msg;
    self->m_msg_hdl.handle_message(&msg);
  }
cleanup:
  delete[] buf->base;
}

void Client::on_write(uv_write_t *write, int status) {
  if (status != 0) {
    // TODO: handle
  }
  delete write;
}

void Client::on_close(uv_handle_t *handle) {
  Client *client = (Client *)handle->data;
  spdlog::info("Disconnencted from server");
  client->m_qui.update_server_connection(false);
  client->m_state = CLIENT_STATE_USERNAME;
  uv_timer_start(&client->m_try_connect, Client::on_try_connect, 1000, 1000);
}

void Client::stop_async_cb(uv_async_t *handle) {
  Client *client = (Client *)handle->data;
  uv_stop(client->m_loop);
}

void Client::enter_lobby_async_cb(uv_async_t *handle) {
  Client *client = (Client *)handle->data;
  client->m_msg_hdl.send_command_enter_lobby();
}

void Client::stop_async() { uv_async_send(&m_stop); }

void Client::enter_lobby_async() { uv_async_send(&m_enter_lobby); }

void Client::on_try_connect(uv_timer_t *timer) {
  Client *client = (Client *)timer->data;
  if (client->m_is_connected) {
    uv_timer_stop(timer);
    return;
  }
  spdlog::info("Connecting to server...");
  uv_tcp_init(client->m_loop, &client->m_connection);
  client->m_connection.data = client;
  uv_ip4_addr(client->m_host, client->m_port, &client->m_server_addr);
  uv_connect_t *connect = new uv_connect_t;
  connect->data = client;
  uv_tcp_connect(connect, &client->m_connection,
                 (struct sockaddr *)&client->m_server_addr, Client::on_connect);
}

}; // namespace C4