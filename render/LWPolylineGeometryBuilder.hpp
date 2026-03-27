#ifndef QADRA_RENDER_LW_POLYLINE_GEOMETRY_BUILDER_HPP
#define QADRA_RENDER_LW_POLYLINE_GEOMETRY_BUILDER_HPP

#include "ArcPass.hpp"
#include "LinePass.hpp"
#include "entity/LWPolyline.hpp"

#include <vector>

namespace Qadra::Render
{
  struct LWPolylineGeometry
  {
    std::vector<ArcPass::Instance> arcInstances;
    std::vector<LinePass::Vertex> lineVertices;
  };

  [[nodiscard]] LWPolylineGeometry buildLWPolylineGeometry ( const Entity::LWPolyline &lwPolyline,
                                                             const glm::vec4 &color,
                                                             std::uint32_t renderKey );
} // namespace Qadra::Render

#endif // QADRA_RENDER_LW_POLYLINE_GEOMETRY_BUILDER_HPP
