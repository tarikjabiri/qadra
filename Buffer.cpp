#include "Buffer.hpp"

namespace Qadra::GL {
  Buffer::Buffer(const Target target) : m_target(target) {
    glCreateBuffers(1, &m_handle);
  }

  Buffer::~Buffer() noexcept {
    glDeleteBuffers(1, &m_handle);
  }

  Buffer::Buffer(Buffer &&rhs) noexcept
    : m_handle(rhs.m_handle), m_target(rhs.m_target) {
    rhs.m_handle = 0;
  }

  Buffer &Buffer::operator=(Buffer &&rhs) noexcept {
    if (this != &rhs) {
      glDeleteBuffers(1, &m_handle);
      m_handle = rhs.m_handle;
      m_target = rhs.m_target;
      rhs.m_handle = 0;
    }

    return *this;
  }

  void Buffer::bind() const {
    glBindBuffer(static_cast<GLenum>(m_target), m_handle);
  }

  void Buffer::unbind() const {
    glBindBuffer(static_cast<GLenum>(m_target), 0);
  }

  void Buffer::allocate(const GLsizeiptr size, Usage usage, const GLvoid *data) const {
    glNamedBufferData(m_handle, size, data, static_cast<GLenum>(usage));
  }

  void Buffer::update(const GLintptr offset, GLsizeiptr size, const GLvoid *data) const {
    glNamedBufferSubData(m_handle, offset, size, data);
  }

  void swap(Buffer &lhs, Buffer &rhs) noexcept {
    std::swap(lhs.m_handle, rhs.m_handle);
    std::swap(lhs.m_target, rhs.m_target);
  }
} // Qadra::GL

