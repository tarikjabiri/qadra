#ifndef QADRA_COMMAND_VIEW_HPP
#define QADRA_COMMAND_VIEW_HPP

#include "HistoryEntry.hpp"
#include "tool/ToolKind.hpp"

#include <string>
#include <vector>

namespace Qadra::Command
{
  struct View
  {
    Tool::ToolKind activeToolKind = Tool::ToolKind::None;
    std::string prompt;
    std::string input;
    std::vector<HistoryEntry> history;
  };
} // namespace Qadra::Command

#endif // QADRA_COMMAND_VIEW_HPP
