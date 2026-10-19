#ifndef C4_COMMON_HPP
#define C4_COMMON_HPP

#include "message.pb.h"
#include <deque>
#include <spdlog/spdlog.h>
#include <uv.h>

namespace C4 {

constexpr size_t MSG_SIZE_NBYTES = 4;
constexpr size_t MSG_MAX_SIZE = 16 * 1024 * 1024;
constexpr size_t USERNAME_MAX_SIZE = 32;

typedef uint32_t game_id_t;
typedef uint32_t client_id_t;

class Server;
class ServerClientConnection;
class Client;
class Game;

class WriteRequest {
public:
  uv_write_t req;
  uv_buf_t buf;
  WriteRequest();
  WriteRequest(Message *msg);
  ~WriteRequest();
};

}; // namespace C4

#endif