#include "server.hpp"

uv_loop_t *g_loop;

using namespace C4;

int main(int argc, char *argv[]) {
  g_loop = uv_default_loop();
  Server server(g_loop, 8080, 128);
  server.run();
  uv_run(g_loop, UV_RUN_DEFAULT);
}