#ifndef QADRA_GL_VERTEXARRAY_HPP
#define QADRA_GL_VERTEXARRAY_HPP

#include "Buffer.hpp"
#include "VertexLayout.hpp"

#include <glad/gl.h>
#include <span>

namespace Qadra::GL
{
  class VertexArray
  {
  public:
    VertexArray ();

    ~VertexArray () noexcept;

    VertexArray ( const VertexArray & ) = delete;

    VertexArray &operator= ( const VertexArray & ) = delete;

    VertexArray ( VertexArray &&rhs ) noexcept;

    VertexArray &operator= ( VertexArray &&rhs ) noexcept;

    void bind () const noexcept;

    static void unbind () noexcept;

    void attribute ( const VertexAttribute &attr ) const noexcept;
    void attributes ( std::span<const VertexAttribute> attrs ) const noexcept;

    void bindingDivisor ( GLuint bindingIndex, GLuint divisor ) const noexcept;
    void bindings ( std::span<const VertexBinding> bindings ) const noexcept;
    void applyLayout ( const VertexLayout &layout ) const noexcept;

    void attachVertexBuffer ( GLuint bindingIndex, const Buffer &buffer, GLintptr offset,
                              GLsizei stride ) const;

    template <typename VertexT>
    void attachVertexBuffer ( const GLuint bindingIndex, const Buffer &buffer,
                              const GLintptr offset = 0 ) const
    {
      attachVertexBuffer ( bindingIndex, buffer, offset, sizeof ( VertexT ) );
    }

    void attachElementBuffer ( const Buffer &buffer ) const;

    [[nodiscard]] GLuint handle () const noexcept { return m_handle; }

    explicit operator bool () const noexcept { return m_handle != 0; }

    friend void swap ( VertexArray &lhs, VertexArray &rhs ) noexcept;

  private:
    GLuint m_handle{};
  };
} // namespace Qadra::GL

#endif // QADRA_GL_VERTEXARRAY_HPP
