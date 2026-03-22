#include "Texture.hpp"

#include <utility>

namespace Qadra::GL {
  Texture::Texture(const GLsizei width, const GLsizei height, const GLenum internalFormat)
    : m_width(width), m_height(height) {
    glCreateTextures(GL_TEXTURE_2D, 1, &m_handle);
    glTextureStorage2D(m_handle, 1, internalFormat, width, height);
    glTextureParameteri(m_handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }

  Texture::~Texture() noexcept {
    glDeleteTextures(1, &m_handle);
  }

  Texture::Texture(Texture &&rhs) noexcept
    : m_handle(rhs.m_handle), m_width(rhs.m_width), m_height(rhs.m_height) {
    rhs.m_handle = 0;
    rhs.m_width = 0;
    rhs.m_height = 0;
  }

  Texture &Texture::operator=(Texture &&rhs) noexcept {
    if (this != &rhs) {
      glDeleteTextures(1, &m_handle);
      m_handle = rhs.m_handle;
      m_width = rhs.m_width;
      m_height = rhs.m_height;
      rhs.m_handle = 0;
      rhs.m_width = 0;
      rhs.m_height = 0;
    }

    return *this;
  }

  void Texture::parameter(const GLenum name, const GLint value) const {
    glTextureParameteri(m_handle, name, value);
  }

  void Texture::upload(const GLint x, const GLint y, const GLsizei width, const GLsizei height, const GLenum format,
                       const GLenum type, const GLvoid *data) const {
    glTextureSubImage2D(m_handle, 0, x, y, width, height, format, type, data);
  }

  void Texture::bind(const GLuint unit) const {
    glBindTextureUnit(unit, m_handle);
  }

  void swap(Texture &lhs, Texture &rhs) noexcept {
    std::swap(lhs.m_handle, rhs.m_handle);
    std::swap(lhs.m_width, rhs.m_width);
    std::swap(lhs.m_height, rhs.m_height);
  }
} // Qadra::GL
