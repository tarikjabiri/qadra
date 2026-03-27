#ifndef QADRA_COMMAND_ARC_STATE_HPP
#define QADRA_COMMAND_ARC_STATE_HPP

#include <glm/glm.hpp>
#include <optional>

namespace Qadra::Command
{
  struct ArcState
  {
    std::optional<glm::dvec2> startPoint;
    std::optional<glm::dvec2> throughPoint;
    std::optional<glm::dvec2> previewPoint;

    [[nodiscard]] bool hasStartPoint () const noexcept { return startPoint.has_value (); }

    [[nodiscard]] bool hasThroughPoint () const noexcept { return throughPoint.has_value (); }
  };
} // namespace Qadra::Command

#endif // QADRA_COMMAND_ARC_STATE_HPP
