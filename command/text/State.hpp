#ifndef QADRA_COMMAND_TEXT_STATE_HPP
#define QADRA_COMMAND_TEXT_STATE_HPP

#include <glm/glm.hpp>
#include <optional>

namespace Qadra::Command
{
  struct TextState
  {
    std::optional<glm::dvec2> insertionPoint;

    [[nodiscard]] bool hasInsertionPoint () const noexcept { return insertionPoint.has_value (); }
  };
} // namespace Qadra::Command

#endif // QADRA_COMMAND_TEXT_STATE_HPP
