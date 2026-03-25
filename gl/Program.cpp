#include "Program.hpp"

#include "glm/gtc/type_ptr.hpp"

namespace Qadra::GL
{
  Program::Program () { m_handle = glCreateProgram (); }

  Program::~Program () noexcept { glDeleteProgram ( m_handle ); }

  Program::Program ( Program &&rhs ) noexcept
      : m_handle ( rhs.m_handle ), m_message ( std::move ( rhs.m_message ) )
  {
    rhs.m_handle = 0;
  }

  Program &Program::operator= ( Program &&rhs ) noexcept
  {
    if ( this != &rhs )
    {
      glDeleteProgram ( m_handle );
      m_handle = rhs.m_handle;
      m_message = std::move ( rhs.m_message );
      rhs.m_handle = 0;
    }

    return *this;
  }

  bool Program::link ( const Shader &vertex, const Shader &fragment )
  {
    glAttachShader ( m_handle, vertex.handle () );
    glAttachShader ( m_handle, fragment.handle () );
    glLinkProgram ( m_handle );

    glDetachShader ( m_handle, vertex.handle () );
    glDetachShader ( m_handle, fragment.handle () );

    GLint status;
    glGetProgramiv ( m_handle, GL_LINK_STATUS, &status );

    if ( status == GL_FALSE )
    {
      GLint length;
      glGetProgramiv ( m_handle, GL_INFO_LOG_LENGTH, &length );
      m_message.resize ( length );
      glGetProgramInfoLog ( m_handle, length, nullptr, m_message.data () );
      return false;
    }

    return true;
  }

  void Program::bind () const noexcept { glUseProgram ( m_handle ); }

  void Program::unbind () noexcept { glUseProgram ( 0 ); }

  void Program::uniform ( const GLchar *name, const int value ) const
  {
    const GLint location = glGetUniformLocation ( m_handle, name );
    glProgramUniform1i ( m_handle, location, value );
  }

  void Program::uniform ( const GLchar *name, const float value ) const
  {
    const GLint location = glGetUniformLocation ( m_handle, name );
    glProgramUniform1f ( m_handle, location, value );
  }

  void Program::uniform ( const GLchar *name, const glm::vec2 &value ) const
  {
    const GLint location = glGetUniformLocation ( m_handle, name );
    glProgramUniform2f ( m_handle, location, value.x, value.y );
  }

  void Program::uniform ( const GLchar *name, const glm::vec3 &value ) const
  {
    const GLint location = glGetUniformLocation ( m_handle, name );
    glProgramUniform3f ( m_handle, location, value.x, value.y, value.z );
  }

  void Program::uniform ( const GLchar *name, const glm::mat4 &value ) const
  {
    const GLint location = glGetUniformLocation ( m_handle, name );
    glProgramUniformMatrix4fv ( m_handle, location, 1, GL_FALSE, glm::value_ptr ( value ) );
  }

  void Program::uniform ( const GLchar *name, const glm::dmat4 &value ) const
  {
    const GLint location = glGetUniformLocation ( m_handle, name );
    glProgramUniformMatrix4dv ( m_handle, location, 1, GL_FALSE, glm::value_ptr ( value ) );
  }

  void swap ( Program &lhs, Program &rhs ) noexcept
  {
    std::swap ( lhs.m_handle, rhs.m_handle );
    std::swap ( lhs.m_message, rhs.m_message );
  }
} // namespace Qadra::GL
