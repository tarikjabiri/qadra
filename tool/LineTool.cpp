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

  ToolEventResult LineTool::onPointerMove ( const ToolContext &, const ToolPointerEvent &event )
  {
    if ( ! hasPendingStart () ) return ToolEventResult::ignored ();

    m_currentPoint = event.worldPosition;
    return ToolEventResult::repaintOnly ();
  }

  ToolPreview LineTool::preview ( const ToolContext & ) const
  {
    if ( ! m_startPoint.has_value () || ! m_currentPoint.has_value () ) return {};

    ToolPreview preview;
    preview.lines.push_back ( ToolPreviewLine{
        .start = *m_startPoint,
        .end = *m_currentPoint,
        .color = { 1.0f, 0.72f, 0.24f, 0.95f },
    } );
    return preview;
  }

  ToolEventResult LineTool::cancel ( const ToolContext & ) { return resetState (); }

  bool LineTool::hasPendingStart () const noexcept { return m_startPoint.has_value (); }

  ToolEventResult LineTool::beginLine ( const ToolPointerEvent &event )
  {
    m_startPoint = event.worldPosition;
    m_currentPoint = event.worldPosition;
    return ToolEventResult::handledAndRepaint ();
  }

  ToolEventResult LineTool::commitLine ( const ToolContext &context, const ToolPointerEvent &event )
  {
    const glm::dvec2 endPoint = event.worldPosition;
    context.document.addLine ( { *m_startPoint, endPoint } );
    m_startPoint = endPoint;
    m_currentPoint = endPoint;
    return ToolEventResult::handledAndRepaint ();
  }

  ToolEventResult LineTool::resetState () noexcept
  {
    if ( ! m_startPoint.has_value () && ! m_currentPoint.has_value () )
      return ToolEventResult::ignored ();

    m_startPoint.reset ();
    m_currentPoint.reset ();
    return ToolEventResult::handledAndRepaint ();
  }
} // namespace Qadra::Tool
