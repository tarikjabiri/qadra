#ifndef QADRA_PREVIEW_RENDERER_HPP
#define QADRA_PREVIEW_RENDERER_HPP

#include "ArcPass.hpp"
#include "Camera.hpp"
#include "EllipsePass.hpp"
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

  struct PreviewArc
  {
    glm::dvec2 center{ 0.0 };
    double radius = 0.0;
    double startAngle = 0.0;
    double sweepAngle = 0.0;
    glm::vec4 color{ 1.0f };
  };

  struct PreviewEllipse
  {
    glm::dvec2 center{ 0.0 };
    glm::dvec2 majorDirection{ 1.0, 0.0 };
    double majorRadius = 0.0;
    double minorRadius = 0.0;
    glm::vec4 color{ 1.0f };
  };

  struct PreviewScene
  {
    std::vector<PreviewLine> lines;
    std::vector<PreviewArc> arcs;
    std::vector<PreviewEllipse> ellipses;
  };

  class PreviewRenderer
  {
  public:
    void init ();
    void sync ( const PreviewScene &preview );
    void draw ( const Core::Camera &camera ) const;

  private:
    std::vector<LinePass::Vertex> m_vertices;
    VertexBatch<LinePass::Vertex> m_batch;
    std::vector<ArcPass::Instance> m_arcInstances;
    VertexBatch<ArcPass::Instance> m_arcBatch;
    std::vector<EllipsePass::Instance> m_ellipseInstances;
    VertexBatch<EllipsePass::Instance> m_ellipseBatch;
    LinePass m_linePass;
    ArcPass m_arcPass;
    EllipsePass m_ellipsePass;
  };
} // namespace Qadra::Render

#endif // QADRA_PREVIEW_RENDERER_HPP
