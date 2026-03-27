#ifndef QADRA_TOOL_TOOL_COMMAND_HPP
#define QADRA_TOOL_TOOL_COMMAND_HPP

#include "command/point/Expression.hpp"

namespace Qadra::Tool
{
  struct ToolCommand
  {
    Command::PointExpression point;
  };
} // namespace Qadra::Tool

#endif // QADRA_TOOL_TOOL_COMMAND_HPP
