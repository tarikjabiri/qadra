#ifndef QADRA_GL_VERTEX_LAYOUT_HPP
#define QADRA_GL_VERTEX_LAYOUT_HPP

#include <glad/gl.h>
#include <span>

namespace Qadra::GL
{
  struct VertexAttribute
  {
    GLuint index{};
    GLint size{};
    GLenum type{};
    GLuint relativeOffset{};
    GLboolean normalized = GL_FALSE;
    bool integer = false;
    GLuint bindingIndex = 0;
  };

  struct VertexBinding
  {
    GLuint bindingIndex = 0;
    GLuint divisor = 0;
  };

  class VertexLayout
  {
  public:
    constexpr VertexLayout () = default;

    constexpr VertexLayout ( std::span<const VertexAttribute> attributes,
                             std::span<const VertexBinding> bindings = {} ) noexcept
        : m_attributes ( attributes ), m_bindings ( bindings )
    {
    }

    [[nodiscard]] constexpr std::span<const VertexAttribute> attributes () const noexcept
    {
      return m_attributes;
    }

    [[nodiscard]] constexpr std::span<const VertexBinding> bindings () const noexcept
    {
      return m_bindings;
    }

    [[nodiscard]] static constexpr VertexAttribute
    attribute ( const GLuint index, const GLint size, const GLenum type,
                const GLuint relativeOffset, const GLuint bindingIndex = 0 ) noexcept
    {
      VertexAttribute attr;
      attr.index = index;
      attr.size = size;
      attr.type = type;
      attr.relativeOffset = relativeOffset;
      attr.bindingIndex = bindingIndex;
      return attr;
    }

    [[nodiscard]] static constexpr VertexAttribute
    normalizedAttribute ( const GLuint index, const GLint size, const GLenum type,
                          const GLuint relativeOffset, const GLuint bindingIndex = 0 ) noexcept
    {
      VertexAttribute attr = attribute ( index, size, type, relativeOffset, bindingIndex );
      attr.normalized = GL_TRUE;
      return attr;
    }

    [[nodiscard]] static constexpr VertexAttribute
    integerAttribute ( const GLuint index, const GLint size, const GLenum type,
                       const GLuint relativeOffset, const GLuint bindingIndex = 0 ) noexcept
    {
      VertexAttribute attr = attribute ( index, size, type, relativeOffset, bindingIndex );
      attr.integer = true;
      return attr;
    }

    [[nodiscard]] static constexpr VertexBinding binding ( const GLuint bindingIndex,
                                                           const GLuint divisor ) noexcept
    {
      VertexBinding value;
      value.bindingIndex = bindingIndex;
      value.divisor = divisor;
      return value;
    }

  private:
    std::span<const VertexAttribute> m_attributes;
    std::span<const VertexBinding> m_bindings;
  };
} // namespace Qadra::GL

#endif // QADRA_GL_VERTEX_LAYOUT_HPP
