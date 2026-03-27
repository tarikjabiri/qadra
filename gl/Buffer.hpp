#ifndef QADRA_GL_BUFFER_HPP
#define QADRA_GL_BUFFER_HPP

#include <glad/gl.h>
#include <span>

namespace Qadra::GL
{
  class Buffer
  {
  public:
    enum class Target : GLenum
    {
      ArrayBuffer = GL_ARRAY_BUFFER,
      DrawIndirectBuffer = GL_DRAW_INDIRECT_BUFFER,
    };

    enum class Usage : GLenum
    {
      StaticDraw = GL_STATIC_DRAW,
      DynamicDraw = GL_DYNAMIC_DRAW,
    };

    explicit Buffer ( Target target = Target::ArrayBuffer );

    ~Buffer () noexcept;

    Buffer ( const Buffer & ) = delete;

    Buffer &operator= ( const Buffer & ) = delete;

    Buffer ( Buffer &&rhs ) noexcept;

    Buffer &operator= ( Buffer &&rhs ) noexcept;

    void bind () const;

    void unbind () const;

    void allocate ( GLsizeiptr size, Usage usage = Usage::StaticDraw,
                    const GLvoid *data = nullptr ) const;

    template <typename T, std::size_t Extent>
    void allocate ( std::span<const T, Extent> data, Usage usage = Usage::StaticDraw ) const
    {
      glNamedBufferData ( m_handle, data.size_bytes (), data.data (),
                          static_cast<GLenum> ( usage ) );
    }

    void update ( GLintptr offset, GLsizeiptr size, const GLvoid *data ) const;

    template <typename T>
    void update ( const GLintptr offset, std::span<const T> data ) const
    {
      glNamedBufferSubData ( m_handle, offset, data.size_bytes (), data.data () );
    }

    [[nodiscard]] GLuint handle () const noexcept { return m_handle; }

    explicit operator bool () const noexcept { return m_handle != 0; }

    friend void swap ( Buffer &lhs, Buffer &rhs ) noexcept;

  private:
    GLuint m_handle{};
    Target m_target{};
  };
} // namespace Qadra::GL

#endif // QADRA_GL_BUFFER_HPP
