#include "client.hpp"

uv_loop_t *g_loop;

using namespace C4;

int main(int argc, char *argv[]) {
  Client client("localhost", 8080);
  client.run();
}