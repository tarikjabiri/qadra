#ifndef QADRA_GL_TEXTURE_HPP
#define QADRA_GL_TEXTURE_HPP

#include <glad/gl.h>

namespace Qadra::GL
{
  class Texture
  {
  public:
    explicit Texture ( GLsizei width, GLsizei height, GLenum internalFormat = GL_RGB8 );

    ~Texture () noexcept;

    Texture ( const Texture & ) = delete;

    Texture &operator= ( const Texture & ) = delete;

    Texture ( Texture &&rhs ) noexcept;

    Texture &operator= ( Texture &&rhs ) noexcept;

    void parameter ( GLenum name, GLint value ) const;

    void upload ( GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type,
                  const GLvoid *data ) const;

    void bind ( GLuint unit = 0 ) const;

    [[nodiscard]] GLuint handle () const noexcept { return m_handle; }

    [[nodiscard]] GLsizei width () const noexcept { return m_width; }

    [[nodiscard]] GLsizei height () const noexcept { return m_height; }

    explicit operator bool () const noexcept { return m_handle != 0; }

    friend void swap ( Texture &lhs, Texture &rhs ) noexcept;

  private:
    GLuint m_handle{};
    GLsizei m_width{};
    GLsizei m_height{};
  };
} // namespace Qadra::GL

#endif // QADRA_GL_TEXTURE_HPP
