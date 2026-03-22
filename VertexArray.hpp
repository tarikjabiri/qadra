#ifndef QADRA_GL_VERTEXARRAY_HPP
#define QADRA_GL_VERTEXARRAY_HPP

#include <glad/gl.h>

#include "Buffer.hpp"

namespace Qadra::GL {
  class VertexArray {
  public:
    struct Attribute {
      GLuint index{};
      GLint size{};
      GLenum type{};
      GLuint relativeOffset{};
      GLboolean normalized = GL_FALSE;
      GLuint bindingIndex = 0;
    };

    VertexArray();

    ~VertexArray() noexcept;

    VertexArray(const VertexArray &) = delete;

    VertexArray &operator=(const VertexArray &) = delete;

    VertexArray(VertexArray &&rhs) noexcept;

    VertexArray &operator=(VertexArray &&rhs) noexcept;

    void bind() const noexcept;

    static void unbind() noexcept;

    void attribute(const Attribute &attr) const noexcept;

    void bindingDivisor(GLuint bindingIndex, GLuint divisor) const noexcept;

    void attachVertexBuffer(GLuint bindingIndex, const Buffer &buffer, GLintptr offset, GLsizei stride) const;

    void attachElementBuffer(const Buffer &buffer) const;

    [[nodiscard]] GLuint handle() const noexcept { return m_handle; }

    explicit operator bool() const noexcept { return m_handle != 0; }

    friend void swap(VertexArray &lhs, VertexArray &rhs) noexcept;

  private:
    GLuint m_handle{};
  };
} // Qadra::GL

#endif // QADRA_GL_VERTEXARRAY_HPP
