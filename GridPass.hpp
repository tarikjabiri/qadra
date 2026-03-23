#ifndef QADRA_RENDER_GRIDPASS_HPP
#define QADRA_RENDER_GRIDPASS_HPP

#include "Buffer.hpp"
#include "Camera.hpp"
#include "Program.hpp"
#include "VertexArray.hpp"

namespace Qadra::Render
{
  class GridPass
  {
  public:
    struct Segment
    {
      glm::dvec2 from;
      glm::dvec2 to;
      glm::vec4 color;
      float lineWidthPixels = 1.0f;
      float antiAliasWidthPixels = 1.0f;
    };

    GridPass () = default;

    void init ( const QString &vertexSource, const QString &fragmentSource );

    void render ( const Core::Camera &camera, const glm::vec2 &viewportSizePixels ) const;

  private:
    GL::Buffer m_vbo;
    GL::VertexArray m_vao;
    GL::Program m_program;
  };
} // namespace Qadra::Render

#endif // QADRA_RENDER_GRIDPASS_HPP
