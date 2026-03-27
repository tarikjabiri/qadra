#ifndef QADRA_COMMAND_POLYLINE_STATE_HPP
#define QADRA_COMMAND_POLYLINE_STATE_HPP

#include "entity/LWPolyline.hpp"

#include <glm/glm.hpp>
#include <optional>
#include <vector>

namespace Qadra::Command
{
  enum class PolylineMode
  {
    Line,
    Arc,
  };

  struct PolylineState
  {
    std::vector<Entity::LWPolylineVertex> vertices;
    std::optional<glm::dvec2> previewPoint;
    std::optional<glm::dvec2> arcThroughPoint;
    PolylineMode mode = PolylineMode::Line;

    [[nodiscard]] bool hasAnchorPoint () const noexcept { return ! vertices.empty (); }

    [[nodiscard]] bool hasArcThroughPoint () const noexcept { return arcThroughPoint.has_value (); }

    [[nodiscard]] const glm::dvec2 &firstPoint () const noexcept { return vertices.front ().point; }

    [[nodiscard]] const glm::dvec2 &lastPoint () const noexcept { return vertices.back ().point; }

    [[nodiscard]] bool canClose () const noexcept { return vertices.size () >= 3; }
  };
} // namespace Qadra::Command

#endif // QADRA_COMMAND_POLYLINE_STATE_HPP
