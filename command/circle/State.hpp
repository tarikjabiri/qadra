#ifndef QADRA_COMMAND_CIRCLE_STATE_HPP
#define QADRA_COMMAND_CIRCLE_STATE_HPP

#include <glm/glm.hpp>
#include <optional>

namespace Qadra::Command
{
  struct CircleState
  {
    std::optional<glm::dvec2> centerPoint;
    std::optional<glm::dvec2> previewPoint;

    [[nodiscard]] bool hasCenterPoint () const noexcept { return centerPoint.has_value (); }
  };
} // namespace Qadra::Command

#endif // QADRA_COMMAND_CIRCLE_STATE_HPP
