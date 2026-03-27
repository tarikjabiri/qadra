#ifndef QADRA_TOOL_COMMAND_RESULT_HPP
#define QADRA_TOOL_COMMAND_RESULT_HPP

#include <string>
#include <utility>

namespace Qadra::Tool
{
  struct CommandResult
  {
    bool accepted = false;
    bool requestRepaint = false;
    std::string message;

    [[nodiscard]] static CommandResult ignored () noexcept { return {}; }

    [[nodiscard]] static CommandResult handled ( std::string message = {} )
    {
      return { true, false, std::move ( message ) };
    }

    [[nodiscard]] static CommandResult handledAndRepaint ( std::string message = {} )
    {
      return { true, true, std::move ( message ) };
    }

    [[nodiscard]] static CommandResult rejected ( std::string message )
    {
      return { false, false, std::move ( message ) };
    }
  };
} // namespace Qadra::Tool

#endif // QADRA_TOOL_COMMAND_RESULT_HPP
