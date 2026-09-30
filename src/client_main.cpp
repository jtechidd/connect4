#include "client.hpp"

uv_loop_t *g_loop;

using namespace C4;

int main(int argc, char *argv[]) {
  g_loop = uv_default_loop();
  Client client(g_loop, "localhost", 8080);
  client.run();
  uv_run(g_loop, UV_RUN_DEFAULT);
}