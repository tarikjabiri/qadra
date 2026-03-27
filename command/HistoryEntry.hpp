#ifndef QADRA_COMMAND_HISTORY_ENTRY_HPP
#define QADRA_COMMAND_HISTORY_ENTRY_HPP

#include <string>
#include <utility>

namespace Qadra::Command
{
  enum class HistoryEntryKind
  {
    Command,
    Info,
    Error
  };

  struct HistoryEntry
  {
    HistoryEntryKind kind = HistoryEntryKind::Info;
    std::string text;

    [[nodiscard]] static HistoryEntry command ( std::string text )
    {
      return { HistoryEntryKind::Command, std::move ( text ) };
    }

    [[nodiscard]] static HistoryEntry info ( std::string text )
    {
      return { HistoryEntryKind::Info, std::move ( text ) };
    }

    [[nodiscard]] static HistoryEntry error ( std::string text )
    {
      return { HistoryEntryKind::Error, std::move ( text ) };
    }
  };
} // namespace Qadra::Command

#endif // QADRA_COMMAND_HISTORY_ENTRY_HPP
