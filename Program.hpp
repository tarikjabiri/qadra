#ifndef QADRA_GL_PROGRAM_HPP
#define QADRA_GL_PROGRAM_HPP

#include "Shader.hpp"

#include <glad/gl.h>
#include <glm/glm.hpp>

namespace Qadra::GL
{
  class Program
  {
  public:
    Program ();

    ~Program () noexcept;

    Program ( const Program & ) = delete;

    Program &operator= ( const Program & ) = delete;

    Program ( Program &&rhs ) noexcept;

    Program &operator= ( Program &&rhs ) noexcept;

    [[nodiscard]] bool link ( const Shader &vertex, const Shader &fragment );

    void bind () const noexcept;

    static void unbind () noexcept;

    void uniform ( const GLchar *name, int value ) const;

    void uniform ( const GLchar *name, float value ) const;

    void uniform ( const GLchar *name, const glm::vec2 &value ) const;

    void uniform ( const GLchar *name, const glm::vec3 &value ) const;

    void uniform ( const GLchar *name, const glm::mat4 &value ) const;

    void uniform ( const GLchar *name, const glm::dmat4 &value ) const;

    [[nodiscard]] GLuint handle () const noexcept { return m_handle; }

    [[nodiscard]] const std::string &message () const noexcept { return m_message; }

    explicit operator bool () const noexcept { return m_handle != 0; }

    friend void swap ( Program &lhs, Program &rhs ) noexcept;

  private:
    GLuint m_handle;
    std::string m_message;
  };
} // namespace Qadra::GL

#endif // QADRA_GL_PROGRAM_HPP
