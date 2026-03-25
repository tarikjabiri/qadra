#ifndef QADRA_GL_SHADER_HPP
#define QADRA_GL_SHADER_HPP

#include <QString>
#include <glad/gl.h>

namespace Qadra::GL
{
  class Shader
  {
  public:
    enum class Type : GLenum
    {
      Vertex = GL_VERTEX_SHADER,
      Fragment = GL_FRAGMENT_SHADER,
      Geometry = GL_GEOMETRY_SHADER,
      Compute = GL_COMPUTE_SHADER,
    };

    explicit Shader ( Type type );

    ~Shader () noexcept;

    Shader ( const Shader & ) = delete;

    Shader &operator= ( const Shader & ) = delete;

    Shader ( Shader &&rhs ) noexcept;

    Shader &operator= ( Shader &&rhs ) noexcept;

    [[nodiscard]] bool compile ( const QString &source );

    [[nodiscard]] GLuint handle () const noexcept { return m_handle; }

    [[nodiscard]] const std::string &message () const noexcept { return m_message; }

    explicit operator bool () const noexcept { return m_handle != 0; }

    friend void swap ( Shader &lhs, Shader &rhs ) noexcept;

  private:
    Type m_type;
    GLuint m_handle;
    std::string m_message;
  };
} // namespace Qadra::GL

#endif // QADRA_GL_SHADER_HPP
