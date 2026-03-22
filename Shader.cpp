#include "Shader.hpp"


namespace Qadra::GL {
  Shader::Shader(const Type type) : m_type(type) {
    m_handle = glCreateShader(static_cast<GLenum>(type));
  }

  Shader::~Shader() noexcept {
    glDeleteShader(m_handle);
  }

  Shader::Shader(Shader &&rhs) noexcept
    : m_type(rhs.m_type), m_handle(rhs.m_handle), m_message(std::move(rhs.m_message)) {
    rhs.m_handle = 0;
  }

  Shader &Shader::operator=(Shader &&rhs) noexcept {
    if (this != &rhs) {
      glDeleteShader(m_handle);
      m_type = rhs.m_type;
      m_handle = rhs.m_handle;
      m_message = std::move(rhs.m_message);
      rhs.m_handle = 0;
    }

    return *this;
  }

  bool Shader::compile(const QString& source) {
    const QByteArray bytes = source.toUtf8();
    const char* src = bytes.constData();
    glShaderSource(m_handle, 1, &src, nullptr);
    glCompileShader(m_handle);

    GLint status;
    glGetShaderiv(m_handle, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
      GLint length;
      glGetShaderiv(m_handle, GL_INFO_LOG_LENGTH, &length);
      m_message.resize(length);
      glGetShaderInfoLog(m_handle, length, nullptr, m_message.data());
      return false;
    }

    return true;
  }

  void swap(Shader &lhs, Shader &rhs) noexcept {
    std::swap(lhs.m_type, rhs.m_type);
    std::swap(lhs.m_handle, rhs.m_handle);
    std::swap(lhs.m_message, rhs.m_message);
  }
} // Qadra::GL
