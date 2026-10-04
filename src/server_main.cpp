#include "server.hpp"

using namespace C4;

int main(int argc, char *argv[]) {
  Server server(8080, 128);
  server.run();
}