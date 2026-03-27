#ifndef QADRA_COMMAND_LINE_STATE_HPP
#define QADRA_COMMAND_LINE_STATE_HPP

#include <cstddef>
#include <glm/glm.hpp>
#include <optional>

namespace Qadra::Command
{
  struct LineState
  {
    std::optional<glm::dvec2> firstPoint;
    std::optional<glm::dvec2> lastPoint;
    std::optional<glm::dvec2> previewPoint;
    std::size_t segmentCount = 0;

    [[nodiscard]] bool hasAnchorPoint () const noexcept { return lastPoint.has_value (); }

    [[nodiscard]] bool canClose () const noexcept
    {
      return firstPoint.has_value () && lastPoint.has_value () && segmentCount >= 2;
    }
  };
} // namespace Qadra::Command

#endif // QADRA_COMMAND_LINE_STATE_HPP
