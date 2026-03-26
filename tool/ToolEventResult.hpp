#ifndef QADRA_TOOL_EVENT_RESULT_HPP
#define QADRA_TOOL_EVENT_RESULT_HPP

namespace Qadra::Tool
{
  struct ToolEventResult
  {
    bool handled = false;
    bool requestRepaint = false;

    [[nodiscard]] static ToolEventResult ignored () noexcept { return {}; }

    [[nodiscard]] static ToolEventResult handledOnly () noexcept { return { true, false }; }

    [[nodiscard]] static ToolEventResult repaintOnly () noexcept { return { false, true }; }

    [[nodiscard]] static ToolEventResult handledAndRepaint () noexcept { return { true, true }; }
  };
} // namespace Qadra::Tool

#endif // QADRA_TOOL_EVENT_RESULT_HPP
