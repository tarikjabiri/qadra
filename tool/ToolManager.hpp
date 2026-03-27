#ifndef QADRA_TOOL_MANAGER_HPP
#define QADRA_TOOL_MANAGER_HPP

#include "CommandResult.hpp"
#include "ToolCommand.hpp"
#include "ToolEventResult.hpp"
#include "ToolKind.hpp"
#include "ToolPointerEvent.hpp"
#include "ToolPreview.hpp"

#include <memory>
#include <string>

namespace Qadra::Tool
{
  struct ToolContext;
  class Tool;

  class ToolManager
  {
  public:
    ToolManager () = default;
    ~ToolManager ();

    ToolManager ( const ToolManager & ) = delete;
    ToolManager &operator= ( const ToolManager & ) = delete;
    ToolManager ( ToolManager && ) noexcept = default;
    ToolManager &operator= ( ToolManager && ) noexcept = default;

    [[nodiscard]] ToolEventResult setActiveTool ( std::unique_ptr<Tool> tool,
                                                  const ToolContext &context );

    [[nodiscard]] ToolEventResult clearActiveTool ( const ToolContext &context );

    [[nodiscard]] ToolEventResult handlePointerPress ( const ToolContext &context,
                                                       const ToolPointerEvent &event );

    [[nodiscard]] ToolEventResult handlePointerMove ( const ToolContext &context,
                                                      const ToolPointerEvent &event );

    [[nodiscard]] ToolEventResult handlePointerRelease ( const ToolContext &context,
                                                         const ToolPointerEvent &event );

    [[nodiscard]] CommandResult handleCommand ( const ToolContext &context,
                                                const ToolCommand &command );

    [[nodiscard]] ToolEventResult cancelActiveTool ( const ToolContext &context );

    [[nodiscard]] ToolKind activeToolKind () const noexcept;

    [[nodiscard]] const Tool *activeTool () const noexcept;

    [[nodiscard]] ToolPreview preview ( const ToolContext &context ) const;

    [[nodiscard]] std::string prompt () const;

  private:
    std::unique_ptr<Tool> m_activeTool;
  };
} // namespace Qadra::Tool

#endif // QADRA_TOOL_MANAGER_HPP
