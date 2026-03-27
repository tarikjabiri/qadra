#ifndef QADRA_COMMAND_ELLIPSE_STATE_HPP
#define QADRA_COMMAND_ELLIPSE_STATE_HPP

#include <glm/glm.hpp>
#include <optional>

namespace Qadra::Command
{
  struct EllipseState
  {
    std::optional<glm::dvec2> centerPoint;
    std::optional<glm::dvec2> majorAxisPoint;
    std::optional<glm::dvec2> previewPoint;

    [[nodiscard]] bool hasCenterPoint () const noexcept { return centerPoint.has_value (); }

    [[nodiscard]] bool hasMajorAxisPoint () const noexcept { return majorAxisPoint.has_value (); }
  };
} // namespace Qadra::Command

#endif // QADRA_COMMAND_ELLIPSE_STATE_HPP
