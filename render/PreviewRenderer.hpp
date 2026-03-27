#ifndef QADRA_PREVIEW_RENDERER_HPP
#define QADRA_PREVIEW_RENDERER_HPP

#include "Camera.hpp"
#include "LinePass.hpp"
#include "VertexBatch.hpp"

#include <glm/glm.hpp>
#include <span>
#include <vector>

namespace Qadra::Render
{
  struct PreviewLine
  {
    glm::dvec2 start{ 0.0 };
    glm::dvec2 end{ 0.0 };
    glm::vec4 color{ 1.0f };
  };

  class PreviewRenderer
  {
  public:
    void init ();
    void sync ( std::span<const PreviewLine> lines );
    void draw ( const Core::Camera &camera ) const;

  private:
    std::vector<LinePass::Vertex> m_vertices;
    VertexBatch<LinePass::Vertex> m_batch;
    LinePass m_linePass;
  };
} // namespace Qadra::Render

#endif // QADRA_PREVIEW_RENDERER_HPP
