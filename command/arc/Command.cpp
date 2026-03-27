#include "Command.hpp"

#include "Document.hpp"

#include <iomanip>
#include <sstream>

namespace
{
  constexpr glm::vec4 kPreviewColor{ 1.0f, 0.72f, 0.24f, 0.95f };

  [[nodiscard]] std::string formatPoint ( const glm::dvec2 &point )
  {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision ( 3 ) << point.x << ", " << point.y;
    return stream.str ();
  }
} // namespace

namespace Qadra::Command
{
  ArcCommand::ArcCommand () : SessionCommand ( Tool::ToolKind::Arc ) { }

  Output ArcCommand::start ( const Context & )
  {
    Output output = Output::handledOnly ( true );
    output.addHistory ( HistoryEntry::command ( "ARC" ) );
    return output;
  }

  Output ArcCommand::submit ( const Context &context, const std::string_view text )
  {
    const auto expression = m_pointParser.parse ( text );
    if ( ! expression.has_value () )
    {
      Output output = Output::handledOnly ( true );
      output.addHistory ( HistoryEntry::error ( "Invalid point expression." ) );
      return output;
    }

    const ResolveResult resolved =
        m_pointResolver.resolve ( *expression, ResolveContext{ basePoint () } );
    if ( ! resolved.ok )
    {
      Output output = Output::handledOnly ( true );
      output.addHistory ( HistoryEntry::error ( resolved.message ) );
      return output;
    }

    return applyWorldPoint ( context, resolved.point, text );
  }

  Output ArcCommand::pointerPress ( const Context &context, const PointerEvent &event )
  {
    if ( event.button != PointerButton::Left ) return Output::ignored ();
    return applyWorldPoint ( context, event.worldPosition );
  }

  Output ArcCommand::pointerMove ( const Context &, const PointerEvent &event )
  {
    if ( ! m_state.hasStartPoint () ) return Output::ignored ();

    m_state.previewPoint = event.worldPosition;
    return Output::handledAndRepaint ();
  }

  Output ArcCommand::finish ( const Context & )
  {
    Output output = Output::finish ( m_state.hasStartPoint (), true );
    output.addHistory ( HistoryEntry::info ( "ARC finished" ) );
    return output;
  }

  Output ArcCommand::cancel ( const Context & )
  {
    Output output = Output::finish ( m_state.hasStartPoint (), true );
    output.addHistory ( HistoryEntry::info ( "ARC canceled" ) );
    return output;
  }

  Preview ArcCommand::preview () const
  {
    Preview preview;

    if ( ! m_state.hasStartPoint () || ! m_state.previewPoint.has_value () ) return preview;

    if ( ! m_state.hasThroughPoint () )
    {
      preview.lines.push_back ( PreviewLine{
          .start = *m_state.startPoint,
          .end = *m_state.previewPoint,
          .color = kPreviewColor,
      } );
      return preview;
    }

    const auto arc = buildArc ( *m_state.previewPoint );
    if ( arc.has_value () )
    {
      preview.arcs.push_back ( PreviewArc{
          .center = arc->center (),
          .radius = arc->radius (),
          .startAngle = arc->startAngle (),
          .sweepAngle = arc->sweepAngle (),
          .color = kPreviewColor,
      } );
      return preview;
    }

    preview.lines.push_back ( PreviewLine{
        .start = *m_state.startPoint,
        .end = *m_state.throughPoint,
        .color = kPreviewColor,
    } );
    preview.lines.push_back ( PreviewLine{
        .start = *m_state.throughPoint,
        .end = *m_state.previewPoint,
        .color = kPreviewColor,
    } );
    return preview;
  }

  std::string ArcCommand::prompt () const
  {
    if ( ! m_state.hasStartPoint () ) return "Specify arc start point";
    if ( ! m_state.hasThroughPoint () ) return "Specify second point of arc";
    return "Specify end point of arc";
  }

  Output ArcCommand::applyWorldPoint ( const Context &context, const glm::dvec2 &point,
                                       const std::string_view sourceText )
  {
    if ( ! m_state.hasStartPoint () ) return beginArc ( point, sourceText );
    if ( ! m_state.hasThroughPoint () ) return setThroughPoint ( point, sourceText );
    return commitArc ( context, point, sourceText );
  }

  Output ArcCommand::beginArc ( const glm::dvec2 &point, const std::string_view sourceText )
  {
    m_state.startPoint = point;
    m_state.previewPoint = point;

    Output output = Output::handledAndRepaint ( true );
    output.addHistory (
        HistoryEntry::info ( "Start point: " + describePoint ( point, sourceText ) ) );
    return output;
  }

  Output ArcCommand::setThroughPoint ( const glm::dvec2 &point, const std::string_view sourceText )
  {
    m_state.throughPoint = point;
    m_state.previewPoint = point;

    Output output = Output::handledAndRepaint ( true );
    output.addHistory (
        HistoryEntry::info ( "Second point: " + describePoint ( point, sourceText ) ) );
    return output;
  }

  Output ArcCommand::commitArc ( const Context &context, const glm::dvec2 &point,
                                 const std::string_view sourceText )
  {
    const auto arc = buildArc ( point );

    if ( ! arc.has_value () )
    {
      m_state.previewPoint = point;

      Output output = Output::handledAndRepaint ( true );
      output.addHistory ( HistoryEntry::error ( "Arc requires three non-collinear points." ) );
      return output;
    }

    context.document.addArc ( {
        .center = arc->center (),
        .radius = arc->radius (),
        .startAngle = arc->startAngle (),
        .sweepAngle = arc->sweepAngle (),
    } );

    Output output = Output::finish ( true, true );
    output.addHistory (
        HistoryEntry::info ( "End point: " + describePoint ( point, sourceText ) ) );
    output.addHistory ( HistoryEntry::info ( "ARC finished" ) );
    return output;
  }

  std::optional<Math::Arc> ArcCommand::buildArc ( const glm::dvec2 &endPoint ) const
  {
    if ( ! m_state.startPoint.has_value () || ! m_state.throughPoint.has_value () )
      return std::nullopt;

    return m_arcBuilder.fromThreePoints ( *m_state.startPoint, *m_state.throughPoint, endPoint );
  }

  std::string ArcCommand::describePoint ( const glm::dvec2 &point,
                                          const std::string_view sourceText )
  {
    const std::string formattedPoint = formatPoint ( point );
    if ( sourceText.empty () ) return formattedPoint;
    return std::string ( sourceText ) + " -> " + formattedPoint;
  }

  std::optional<glm::dvec2> ArcCommand::basePoint () const noexcept
  {
    if ( m_state.throughPoint.has_value () ) return m_state.throughPoint;
    return m_state.startPoint;
  }
} // namespace Qadra::Command
