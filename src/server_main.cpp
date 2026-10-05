#include "server.hpp"

int main(int argc, char *argv[]) {
  spdlog::set_level(spdlog::level::debug);
  C4::Server server(8080, 128);
  server.run();
}