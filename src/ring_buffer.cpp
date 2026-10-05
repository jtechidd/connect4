#include "ring_buffer.hpp"

namespace C4 {

const size_t DEFAULT_RING_BUFFER_CAPACITY = 8;

RingBuffer::RingBuffer(size_t cap) {
  if (cap == 0) {
    throw std::invalid_argument{
        "Ring buffer capacity must be greater than zero"};
  }
  m_buf = (uint8_t *)malloc(cap);
  if (!m_buf)
    throw std::bad_alloc{};
  m_size = 0;
  m_cap = cap;
  m_read_pos = 0;
  m_write_pos = 0;
}

RingBuffer::~RingBuffer() { free(m_buf); }

size_t RingBuffer::free_space() { return m_cap - m_size; }

int RingBuffer::grow() {
  if (m_cap > (SIZE_MAX >> 1)) {
    return -1;
  }
  size_t new_cap = m_cap << 1;
  uint8_t *new_buf = (uint8_t *)realloc(m_buf, new_cap);
  if (!new_buf) {
    return -1;
  }
  size_t first = std::min(m_cap - m_read_pos, m_size);
  memmove(new_buf + new_cap - first, new_buf + m_read_pos, first);
  m_buf = new_buf;
  m_cap = new_cap;
  m_read_pos = new_cap - first;
  m_write_pos = (m_read_pos + m_size) % new_cap;
  return 0;
}

int RingBuffer::write(void *src, size_t len) {
  while (free_space() < len)
    if (grow() < 0)
      return -1;
  size_t first = std::min(m_cap - m_write_pos, len);
  memcpy(m_buf + m_write_pos, src, first);
  memcpy(m_buf, (uint8_t *)src + first, len - first);
  m_write_pos = (m_write_pos + len) % m_cap;
  m_size += len;
  return 0;
}

size_t RingBuffer::peek(void *dst, size_t cnt, size_t len) {
  size_t n = std::min(std::min(len, m_size), cnt);
  size_t first = std::min(m_cap - m_read_pos, n);
  memcpy(dst, m_buf + m_read_pos, first);
  memcpy((uint8_t *)dst + first, m_buf, n - first);
  return n;
}

size_t RingBuffer::consume(size_t len) {
  size_t n = std::min(len, m_size);
  m_read_pos = (m_read_pos + n) % m_cap;
  m_size -= n;
  return n;
}

size_t RingBuffer::read(void *dst, size_t cnt, size_t len) {
  size_t n = peek(dst, cnt, len);
  return consume(n);
}

uint8_t *RingBuffer::get_read_ptr() { return m_buf + m_read_pos; }
}; // namespace C4