#include "Command.hpp"

#include "Document.hpp"
#include "math/Bulge.hpp"

#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>

namespace
{
  constexpr glm::vec4 kPreviewColor{ 1.0f, 0.72f, 0.24f, 0.95f };
  constexpr double kPointEpsilon = 1e-9;

  [[nodiscard]] std::string_view trim ( std::string_view text ) noexcept
  {
    while ( ! text.empty () && std::isspace ( static_cast<unsigned char> ( text.front () ) ) )
      text.remove_prefix ( 1 );

    while ( ! text.empty () && std::isspace ( static_cast<unsigned char> ( text.back () ) ) )
      text.remove_suffix ( 1 );

    return text;
  }

  [[nodiscard]] std::string toLower ( const std::string_view text )
  {
    std::string normalized;
    normalized.reserve ( text.size () );

    for ( const char ch : text )
      normalized.push_back (
          static_cast<char> ( std::tolower ( static_cast<unsigned char> ( ch ) ) ) );

    return normalized;
  }

  [[nodiscard]] bool isArcAlias ( const std::string_view text )
  {
    const std::string normalized = toLower ( trim ( text ) );
    return normalized == "a" || normalized == "arc";
  }

  [[nodiscard]] bool isLineAlias ( const std::string_view text )
  {
    const std::string normalized = toLower ( trim ( text ) );
    return normalized == "l" || normalized == "line";
  }

  [[nodiscard]] bool isCloseAlias ( const std::string_view text )
  {
    const std::string normalized = toLower ( trim ( text ) );
    return normalized == "c" || normalized == "close";
  }

  [[nodiscard]] bool samePoint ( const glm::dvec2 &lhs, const glm::dvec2 &rhs ) noexcept
  {
    const glm::dvec2 delta = lhs - rhs;
    return glm::dot ( delta, delta ) <= kPointEpsilon * kPointEpsilon;
  }

  [[nodiscard]] std::string formatPoint ( const glm::dvec2 &point )
  {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision ( 3 ) << point.x << ", " << point.y;
    return stream.str ();
  }
} // namespace

namespace Qadra::Command
{
  PolylineCommand::PolylineCommand () : SessionCommand ( Tool::ToolKind::Polyline ) { }

  Output PolylineCommand::start ( const Context & )
  {
    Output output = Output::handledOnly ( true );
    output.addHistory ( HistoryEntry::command ( "PLINE" ) );
    return output;
  }

  Output PolylineCommand::submit ( const Context &context, const std::string_view text )
  {
    if ( isArcAlias ( text ) ) return switchMode ( PolylineMode::Arc );
    if ( isLineAlias ( text ) ) return switchMode ( PolylineMode::Line );

    if ( isCloseAlias ( text ) )
    {
      if ( ! m_state.canClose () )
      {
        Output output = Output::handledOnly ( true );
        output.addHistory ( HistoryEntry::error ( "Close requires at least two segments." ) );
        return output;
      }

      if ( m_state.mode == PolylineMode::Arc )
      {
        Output output = Output::handledOnly ( true );
        output.addHistory ( HistoryEntry::error (
            m_state.hasArcThroughPoint ()
                ? "Specify endpoint of arc or switch to [Line] before Close."
                : "Switch to [Line] before Close." ) );
        return output;
      }

      return closePolyline ( context );
    }

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

  Output PolylineCommand::pointerPress ( const Context &context, const PointerEvent &event )
  {
    if ( event.button != PointerButton::Left ) return Output::ignored ();
    return applyWorldPoint ( context, event.worldPosition );
  }

  Output PolylineCommand::pointerMove ( const Context &, const PointerEvent &event )
  {
    if ( ! m_state.hasAnchorPoint () ) return Output::ignored ();

    m_state.previewPoint = event.worldPosition;
    return Output::handledAndRepaint ();
  }

  Output PolylineCommand::finish ( const Context &context )
  {
    return finishPolyline ( context, false );
  }

  Output PolylineCommand::cancel ( const Context & )
  {
    Output output = Output::finish ( m_state.hasAnchorPoint (), true );
    output.addHistory ( HistoryEntry::info ( "PLINE canceled" ) );
    return output;
  }

  Preview PolylineCommand::preview () const
  {
    Preview preview;
    if ( ! m_state.hasAnchorPoint () || ! m_state.previewPoint.has_value () ) return preview;

    appendCommittedPreviewSegments ( preview );

    if ( m_state.mode == PolylineMode::Line )
    {
      preview.lines.push_back ( PreviewLine{
          .start = m_state.lastPoint (),
          .end = *m_state.previewPoint,
          .color = kPreviewColor,
      } );
      return preview;
    }

    if ( ! m_state.hasArcThroughPoint () )
    {
      preview.lines.push_back ( PreviewLine{
          .start = m_state.lastPoint (),
          .end = *m_state.previewPoint,
          .color = kPreviewColor,
      } );
      return preview;
    }

    const auto arc = buildArcSegment ( *m_state.previewPoint );
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
        .start = m_state.lastPoint (),
        .end = *m_state.arcThroughPoint,
        .color = kPreviewColor,
    } );
    preview.lines.push_back ( PreviewLine{
        .start = *m_state.arcThroughPoint,
        .end = *m_state.previewPoint,
        .color = kPreviewColor,
    } );
    return preview;
  }

  std::string PolylineCommand::prompt () const
  {
    if ( ! m_state.hasAnchorPoint () ) return "Specify first point";

    if ( m_state.mode == PolylineMode::Line )
    {
      if ( m_state.canClose () ) return "Specify next point, [Arc/Close], or press Enter to finish";
      return "Specify next point or [Arc]";
    }

    if ( ! m_state.hasArcThroughPoint () ) return "Specify second point of arc or [Line]";
    return "Specify endpoint of arc or [Line]";
  }

  Output PolylineCommand::applyWorldPoint ( const Context &context, const glm::dvec2 &point,
                                            const std::string_view sourceText )
  {
    if ( ! m_state.hasAnchorPoint () ) return beginPolyline ( point, sourceText );
    if ( m_state.mode == PolylineMode::Line ) return commitLineVertex ( point, sourceText );
    if ( ! m_state.hasArcThroughPoint () ) return setArcThroughPoint ( point, sourceText );
    return commitArcVertex ( point, sourceText );
  }

  Output PolylineCommand::beginPolyline ( const glm::dvec2 &point,
                                          const std::string_view sourceText )
  {
    m_state.vertices.clear ();
    m_state.vertices.push_back ( { .point = point, .bulge = 0.0 } );
    m_state.previewPoint = point;
    m_state.arcThroughPoint.reset ();
    m_state.mode = PolylineMode::Line;

    Output output = Output::handledAndRepaint ( true );
    output.addHistory (
        HistoryEntry::info ( "First point: " + describePoint ( point, sourceText ) ) );
    return output;
  }

  Output PolylineCommand::commitLineVertex ( const glm::dvec2 &point,
                                             const std::string_view sourceText )
  {
    if ( samePoint ( point, m_state.lastPoint () ) )
    {
      Output output = Output::handledOnly ( true );
      output.addHistory ( HistoryEntry::error ( "Next point must differ from previous point." ) );
      return output;
    }

    m_state.vertices.back ().bulge = 0.0;
    m_state.vertices.push_back ( { .point = point, .bulge = 0.0 } );
    m_state.previewPoint = point;

    Output output = Output::handledAndRepaint ( true );
    output.addHistory (
        HistoryEntry::info ( "Next point: " + describePoint ( point, sourceText ) ) );
    return output;
  }

  Output PolylineCommand::setArcThroughPoint ( const glm::dvec2 &point,
                                               const std::string_view sourceText )
  {
    if ( samePoint ( point, m_state.lastPoint () ) )
    {
      Output output = Output::handledOnly ( true );
      output.addHistory (
          HistoryEntry::error ( "Second point of arc must differ from previous point." ) );
      return output;
    }

    m_state.arcThroughPoint = point;
    m_state.previewPoint = point;

    Output output = Output::handledAndRepaint ( true );
    output.addHistory (
        HistoryEntry::info ( "Second point: " + describePoint ( point, sourceText ) ) );
    return output;
  }

  Output PolylineCommand::commitArcVertex ( const glm::dvec2 &point,
                                            const std::string_view sourceText )
  {
    const auto arc = buildArcSegment ( point );
    if ( ! arc.has_value () )
    {
      m_state.previewPoint = point;

      Output output = Output::handledAndRepaint ( true );
      output.addHistory (
          HistoryEntry::error ( "Arc segment requires three non-collinear points." ) );
      return output;
    }

    m_state.vertices.back ().bulge = Math::bulgeFromArc ( *arc );
    m_state.vertices.push_back ( { .point = point, .bulge = 0.0 } );
    m_state.previewPoint = point;
    m_state.arcThroughPoint.reset ();

    Output output = Output::handledAndRepaint ( true );
    output.addHistory (
        HistoryEntry::info ( "Arc endpoint: " + describePoint ( point, sourceText ) ) );
    return output;
  }

  Output PolylineCommand::switchMode ( const PolylineMode mode )
  {
    if ( ! m_state.hasAnchorPoint () )
    {
      Output output = Output::handledOnly ( true );
      output.addHistory (
          HistoryEntry::error ( "Specify the first point before switching modes." ) );
      return output;
    }

    if ( m_state.mode == mode && ( mode != PolylineMode::Arc || ! m_state.hasArcThroughPoint () ) )
      return Output::handledOnly ();

    m_state.mode = mode;
    if ( mode == PolylineMode::Line ) m_state.arcThroughPoint.reset ();

    Output output = Output::handledOnly ( true );
    output.addHistory (
        HistoryEntry::info ( mode == PolylineMode::Arc ? "Mode: Arc" : "Mode: Line" ) );
    return output;
  }

  Output PolylineCommand::closePolyline ( const Context &context )
  {
    if ( samePoint ( m_state.lastPoint (), m_state.firstPoint () ) )
    {
      if ( m_state.vertices.size () > 1 ) m_state.vertices.pop_back ();
      return finishPolyline ( context, true );
    }

    m_state.vertices.back ().bulge = 0.0;
    return finishPolyline ( context, true );
  }

  Output PolylineCommand::finishPolyline ( const Context &context, const bool closed )
  {
    Output output = Output::finish ( m_state.hasAnchorPoint (), true );

    if ( m_state.vertices.size () >= 2 )
    {
      context.document.addLWPolyline ( {
          .vertices = m_state.vertices,
          .closed = closed,
      } );
    }

    output.addHistory ( HistoryEntry::info ( closed ? "PLINE closed" : "PLINE finished" ) );
    return output;
  }

  std::optional<Math::Arc> PolylineCommand::buildArcSegment ( const glm::dvec2 &endPoint ) const
  {
    if ( ! m_state.hasAnchorPoint () || ! m_state.hasArcThroughPoint () ) return std::nullopt;
    return m_arcBuilder.fromThreePoints ( m_state.lastPoint (), *m_state.arcThroughPoint,
                                          endPoint );
  }

  void PolylineCommand::appendCommittedPreviewSegments ( Preview &preview ) const
  {
    const auto &vertices = m_state.vertices;
    if ( vertices.size () < 2 ) return;

    for ( std::size_t index = 0; index + 1 < vertices.size (); ++index )
    {
      const auto &start = vertices[index];
      const auto &end = vertices[index + 1];
      const auto arc = Math::arcFromBulge ( start.point, end.point, start.bulge );

      if ( arc.has_value () )
      {
        preview.arcs.push_back ( PreviewArc{
            .center = arc->center (),
            .radius = arc->radius (),
            .startAngle = arc->startAngle (),
            .sweepAngle = arc->sweepAngle (),
            .color = kPreviewColor,
        } );
        continue;
      }

      preview.lines.push_back ( PreviewLine{
          .start = start.point,
          .end = end.point,
          .color = kPreviewColor,
      } );
    }
  }

  std::optional<glm::dvec2> PolylineCommand::basePoint () const noexcept
  {
    if ( m_state.hasArcThroughPoint () ) return m_state.arcThroughPoint;
    if ( m_state.hasAnchorPoint () ) return m_state.lastPoint ();
    return std::nullopt;
  }

  std::string PolylineCommand::describePoint ( const glm::dvec2 &point,
                                               const std::string_view sourceText )
  {
    const std::string formattedPoint = formatPoint ( point );
    if ( sourceText.empty () ) return formattedPoint;
    return std::string ( sourceText ) + " -> " + formattedPoint;
  }
} // namespace Qadra::Command
