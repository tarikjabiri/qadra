#ifndef QADRA_TOOL_POINTER_EVENT_HPP
#define QADRA_TOOL_POINTER_EVENT_HPP

#include <glm/glm.hpp>

namespace Qadra::Tool
{
  struct ToolModifiers
  {
    bool shift = false;
    bool control = false;
    bool alt = false;
  };

  enum class ToolPointerButton
  {
    None,
    Left,
    Middle,
    Right
  };

  struct ToolPointerEvent
  {
    glm::dvec2 screenPosition{ 0.0 };
    glm::dvec2 worldPosition{ 0.0 };
    ToolPointerButton button = ToolPointerButton::None;
    ToolModifiers modifiers{};
  };
} // namespace Qadra::Tool

#endif // QADRA_TOOL_POINTER_EVENT_HPP
