#include "LineTool.hpp"

#include "Document.hpp"

namespace Qadra::Tool
{
  LineTool::LineTool () : Tool ( ToolKind::Line ) { }

  void LineTool::deactivate ( const ToolContext & ) { resetState (); }

  ToolEventResult LineTool::onPointerPress ( const ToolContext &context,
                                             const ToolPointerEvent &event )
  {
    if ( event.button != ToolPointerButton::Left ) return ToolEventResult::ignored ();

    if ( ! hasPendingStart () ) return beginLine ( event );

    return commitLine ( context, event );
  }

  ToolEventResult LineTool::cancel ( const ToolContext & ) { return resetState (); }

  bool LineTool::hasPendingStart () const noexcept { return m_startPoint.has_value (); }

  ToolEventResult LineTool::beginLine ( const ToolPointerEvent &event )
  {
    m_startPoint = event.worldPosition;
    return ToolEventResult::handledOnly ();
  }

  ToolEventResult LineTool::commitLine ( const ToolContext &context, const ToolPointerEvent &event )
  {
    context.document.addLine ( { *m_startPoint, event.worldPosition } );
    m_startPoint.reset ();
    return ToolEventResult::handledAndRepaint ();
  }

  ToolEventResult LineTool::resetState () noexcept
  {
    if ( ! m_startPoint.has_value () ) return ToolEventResult::ignored ();

    m_startPoint.reset ();
    return ToolEventResult::handledOnly ();
  }
} // namespace Qadra::Tool
