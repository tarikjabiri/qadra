#ifndef QADRA_COMMAND_POINTER_EVENT_HPP
#define QADRA_COMMAND_POINTER_EVENT_HPP

#include <glm/glm.hpp>

namespace Qadra::Command
{
  struct PointerModifiers
  {
    bool shift = false;
    bool control = false;
    bool alt = false;
  };

  enum class PointerButton
  {
    None,
    Left,
    Middle,
    Right
  };

  struct PointerEvent
  {
    glm::dvec2 screenPosition{ 0.0 };
    glm::dvec2 worldPosition{ 0.0 };
    PointerButton button = PointerButton::None;
    PointerModifiers modifiers{};
  };
} // namespace Qadra::Command

#endif // QADRA_COMMAND_POINTER_EVENT_HPP
