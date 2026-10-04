#include "common.hpp"

using namespace C4;

const size_t C4::MSG_SIZE_NBYTES = 4;
const size_t C4::MSG_MAX_SIZE = 16 * 1024 * 1024;

WriteRequest::WriteRequest() {};

WriteRequest::WriteRequest(Message *msg) {
  std::string str = msg->SerializeAsString();
  uint32_t len = str.length();
  uint32_t len_net = htonl(len);
  buf.base = (char *)malloc(MSG_SIZE_NBYTES + len);
  buf.len = MSG_SIZE_NBYTES + len;
  memcpy(buf.base, &len_net, sizeof(uint32_t));
  strncpy(buf.base + MSG_SIZE_NBYTES, str.c_str(), len);
}

WriteRequest::~WriteRequest() { free(buf.base); };