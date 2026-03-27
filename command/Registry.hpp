#ifndef QADRA_COMMAND_REGISTRY_HPP
#define QADRA_COMMAND_REGISTRY_HPP

#include "tool/ToolKind.hpp"

#include <memory>
#include <optional>
#include <string_view>

namespace Qadra::Command
{
  class SessionCommand;

  class Registry final
  {
  public:
    [[nodiscard]] std::unique_ptr<SessionCommand> create ( Tool::ToolKind kind ) const;

    [[nodiscard]] std::optional<Tool::ToolKind> resolveAlias ( std::string_view text ) const;
  };
} // namespace Qadra::Command

#endif // QADRA_COMMAND_REGISTRY_HPP
