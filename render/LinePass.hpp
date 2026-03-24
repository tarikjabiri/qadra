#ifndef QADRA_LINE_PASS_HPP
#define QADRA_LINE_PASS_HPP

#include "Buffer.hpp"
#include "Camera.hpp"
#include "Program.hpp"
#include "VertexArray.hpp"

#include <glm/glm.hpp>
#include <span>

namespace Qadra::Render
{
  class LinePass
  {
  public:
    struct Vertex
    {
      glm::dvec2 position;
      glm::vec4 color;
      float depth;
    };

    LinePass () = default;

    void init ( const QString &vertexSource, const QString &fragmentSource );

    void upload ( std::span<const Vertex> vertices );

    void render ( const Core::Camera &camera ) const;

  private:
    GL::Buffer m_vbo;
    GL::VertexArray m_vao;
    GL::Program m_program;
    std::size_t m_vertexCount = 0;
    bool m_bufferDirty = true;
  };
} // namespace Qadra::Render

#endif // QADRA_LINE_PASS_HPP
