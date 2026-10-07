#include "client.hpp"

int main(int argc, char *argv[]) {
  spdlog::set_level(spdlog::level::debug);
  C4::Client client(argc, argv,"localhost", 8080);
  client.run();
}