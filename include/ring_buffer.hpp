#ifndef C4_RING_BUFFER_HPP
#define C4_RING_BUFFER_HPP

#include "common.hpp"

#define DEFAULT_RING_BUFFER_CAPACITY 4096

class RingBuffer {
public:
  uint8_t *m_buf;
  size_t m_size;
  size_t m_cap;
  size_t m_read_pos;
  size_t m_write_pos;

  RingBuffer(size_t cap = DEFAULT_RING_BUFFER_CAPACITY);
  ~RingBuffer();

  size_t free();
  int grow();
  int write(void *src, size_t len);
  size_t peek(void *dst, size_t cnt, size_t len);
  size_t consume(size_t len);
  size_t read(void *dst, size_t cnt, size_t len);
  uint8_t* get_read_ptr();
};

#endif
