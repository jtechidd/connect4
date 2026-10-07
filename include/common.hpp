#ifndef C4_COMMON_HPP
#define C4_COMMON_HPP

#include "message.pb.h"
#include <spdlog/spdlog.h>
#include <uv.h>

namespace C4 {

extern uv_loop_t *g_uv_loop;
extern uint32_t g_client_cid, g_game_cid;
extern const size_t MSG_SIZE_NBYTES;
extern const size_t MSG_MAX_SIZE;
constexpr size_t USERNAME_MAX_SIZE = 32;

typedef uint32_t game_id_t;
typedef uint32_t client_id_t;

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