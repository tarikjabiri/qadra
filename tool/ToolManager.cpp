#include "ToolManager.hpp"

#include "Tool.hpp"

#include <utility>

namespace Qadra::Tool
{
  ToolManager::~ToolManager () = default;

  ToolEventResult ToolManager::setActiveTool ( std::unique_ptr<Tool> tool,
                                               const ToolContext &context )
  {
    if ( ! m_activeTool && ! tool ) return ToolEventResult::ignored ();

    if ( m_activeTool ) m_activeTool->deactivate ( context );

    m_activeTool = std::move ( tool );

    if ( m_activeTool ) m_activeTool->activate ( context );

    return ToolEventResult::handledAndRepaint ();
  }

  ToolEventResult ToolManager::clearActiveTool ( const ToolContext &context )
  {
    if ( ! m_activeTool ) return ToolEventResult::ignored ();

    m_activeTool->deactivate ( context );
    m_activeTool.reset ();

    return ToolEventResult::handledAndRepaint ();
  }

  ToolEventResult ToolManager::handlePointerPress ( const ToolContext &context,
                                                    const ToolPointerEvent &event )
  {
    if ( ! m_activeTool ) return ToolEventResult::ignored ();
    return m_activeTool->onPointerPress ( context, event );
  }

  ToolEventResult ToolManager::handlePointerMove ( const ToolContext &context,
                                                   const ToolPointerEvent &event )
  {
    if ( ! m_activeTool ) return ToolEventResult::ignored ();
    return m_activeTool->onPointerMove ( context, event );
  }

  ToolEventResult ToolManager::handlePointerRelease ( const ToolContext &context,
                                                      const ToolPointerEvent &event )
  {
    if ( ! m_activeTool ) return ToolEventResult::ignored ();
    return m_activeTool->onPointerRelease ( context, event );
  }

  ToolEventResult ToolManager::cancelActiveTool ( const ToolContext &context )
  {
    if ( ! m_activeTool ) return ToolEventResult::ignored ();
    return m_activeTool->cancel ( context );
  }

  ToolKind ToolManager::activeToolKind () const noexcept
  {
    return m_activeTool ? m_activeTool->kind () : ToolKind::None;
  }

  const Tool *ToolManager::activeTool () const noexcept { return m_activeTool.get (); }
} // namespace Qadra::Tool
