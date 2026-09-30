#include "common.hpp"

using namespace C4;

WriteRequest::WriteRequest() {};

WriteRequest::WriteRequest(Message *msg) {
  std::string str = msg->SerializeAsString();
  uint32_t len = str.length();
  uint32_t len_net = htonl(len);
  buf.base = (char *)std::malloc(len + 4);
  buf.len = len + 4;
  std::memcpy(buf.base, &len_net, sizeof(uint32_t));
  std::strncpy(buf.base + 4, str.c_str(), len);
}

WriteRequest::~WriteRequest() { std::free(buf.base); };