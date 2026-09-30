#ifndef C4_COMMON_HPP
#define C4_COMMON_HPP

#include "message.pb.h"
#include <cstdlib>
#include <cstring>
#include <map>
#include <spdlog/spdlog.h>
#include <string>
#include <uv.h>

extern uv_loop_t *g_loop;

namespace C4 {
extern uint64_t g_session_cid, g_game_cid;

typedef uint64_t game_id_t;
typedef uint64_t session_id_t;
typedef session_id_t client_id_t;

class Server;
class ServerSession;
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