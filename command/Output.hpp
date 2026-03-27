#ifndef QADRA_COMMAND_OUTPUT_HPP
#define QADRA_COMMAND_OUTPUT_HPP

#include "HistoryEntry.hpp"

#include <utility>
#include <vector>

namespace Qadra::Command
{
  struct Output
  {
    bool handled = false;
    bool requestRepaint = false;
    bool viewChanged = false;
    bool finishCommand = false;
    std::vector<HistoryEntry> historyEntries;

    [[nodiscard]] static Output ignored () noexcept { return {}; }

    [[nodiscard]] static Output handledOnly ( const bool viewChanged = false ) noexcept
    {
      return { true, false, viewChanged, false, {} };
    }

    [[nodiscard]] static Output handledAndRepaint ( const bool viewChanged = false ) noexcept
    {
      return { true, true, viewChanged, false, {} };
    }

    [[nodiscard]] static Output finish ( const bool requestRepaint = false,
                                         const bool viewChanged = true ) noexcept
    {
      return { true, requestRepaint, viewChanged, true, {} };
    }

    void addHistory ( HistoryEntry entry )
    {
      viewChanged = true;
      historyEntries.push_back ( std::move ( entry ) );
    }
  };
} // namespace Qadra::Command

#endif // QADRA_COMMAND_OUTPUT_HPP
