#include "LineTool.hpp"

#include "Document.hpp"
#include "command/point/Resolver.hpp"

namespace Qadra::Tool
{
  LineTool::LineTool () : Tool ( ToolKind::Line ) { }

  void LineTool::deactivate ( const ToolContext & ) { resetState (); }

  ToolEventResult LineTool::onPointerPress ( const ToolContext &context,
                                             const ToolPointerEvent &event )
  {
    if ( event.button != ToolPointerButton::Left ) return ToolEventResult::ignored ();
    return applyWorldPoint ( context, event.worldPosition );
  }

  ToolEventResult LineTool::onPointerMove ( const ToolContext &, const ToolPointerEvent &event )
  {
    if ( ! hasPendingStart () ) return ToolEventResult::ignored ();

    m_currentPoint = event.worldPosition;
    return ToolEventResult::repaintOnly ();
  }

  CommandResult LineTool::onCommand ( const ToolContext &context, const ToolCommand &command )
  {
    const Command::PointResolver resolver;
    const Command::ResolveResult resolved =
        resolver.resolve ( command.point, Command::ResolveContext{ commandBasePoint () } );

    if ( ! resolved.ok ) return CommandResult::rejected ( resolved.message );

    const ToolEventResult result = applyWorldPoint ( context, resolved.point );
    if ( ! result.handled )
      return CommandResult::rejected ( "Line tool could not accept the point." );
    return result.requestRepaint ? CommandResult::handledAndRepaint () : CommandResult::handled ();
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

  std::string LineTool::prompt () const
  {
    return hasPendingStart () ? "LINE: Specify next point or press Enter to finish"
                              : "LINE: Specify first point";
  }

  ToolEventResult LineTool::cancel ( const ToolContext & ) { return resetState (); }

  bool LineTool::hasPendingStart () const noexcept { return m_startPoint.has_value (); }

  ToolEventResult LineTool::applyWorldPoint ( const ToolContext &context, const glm::dvec2 &point )
  {
    if ( ! hasPendingStart () ) return beginLine ( point );
    return commitLine ( context, point );
  }

  ToolEventResult LineTool::beginLine ( const glm::dvec2 &point )
  {
    m_startPoint = point;
    m_currentPoint = point;
    return ToolEventResult::handledAndRepaint ();
  }

  ToolEventResult LineTool::commitLine ( const ToolContext &context, const glm::dvec2 &point )
  {
    context.document.addLine ( { *m_startPoint, point } );
    m_startPoint = point;
    m_currentPoint = point;
    return ToolEventResult::handledAndRepaint ();
  }

  std::optional<glm::dvec2> LineTool::commandBasePoint () const noexcept { return m_startPoint; }

  ToolEventResult LineTool::resetState () noexcept
  {
    if ( ! m_startPoint.has_value () && ! m_currentPoint.has_value () )
      return ToolEventResult::ignored ();

    m_startPoint.reset ();
    m_currentPoint.reset ();
    return ToolEventResult::handledAndRepaint ();
  }
} // namespace Qadra::Tool
