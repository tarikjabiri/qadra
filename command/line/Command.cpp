#include "Command.hpp"

#include "cad/history/DocumentEditor.hpp"

#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>

namespace
{
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

  [[nodiscard]] bool isCloseAlias ( const std::string_view text )
  {
    const std::string normalized = toLower ( trim ( text ) );
    return normalized == "c" || normalized == "close";
  }

  [[nodiscard]] bool samePoint ( const glm::dvec2 &lhs, const glm::dvec2 &rhs ) noexcept
  {
    return lhs.x == rhs.x && lhs.y == rhs.y;
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
  LineCommand::LineCommand () : SessionCommand ( Tool::ToolKind::Line ) { }

  Output LineCommand::start ( const Context & )
  {
    Output output = Output::handledOnly ( true );
    output.addHistory ( HistoryEntry::command ( "LINE" ) );
    return output;
  }

  Output LineCommand::submit ( const Context &context, const std::string_view text )
  {
    if ( isCloseAlias ( text ) )
    {
      if ( ! m_state.canClose () )
      {
        Output output = Output::handledOnly ( true );
        output.addHistory (
            HistoryEntry::error ( "Close requires at least two committed segments." ) );
        return output;
      }

      return closeLine ( context );
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

  Output LineCommand::pointerPress ( const Context &context, const PointerEvent &event )
  {
    if ( event.button != PointerButton::Left ) return Output::ignored ();
    return applyWorldPoint ( context, event.worldPosition );
  }

  Output LineCommand::pointerMove ( const Context &, const PointerEvent &event )
  {
    if ( ! m_state.hasAnchorPoint () ) return Output::ignored ();

    m_state.previewPoint = event.worldPosition;
    return Output::handledAndRepaint ();
  }

  Output LineCommand::finish ( const Context & )
  {
    Output output = Output::finish ( m_state.hasAnchorPoint (), true );
    output.addHistory ( HistoryEntry::info ( "LINE finished" ) );
    return output;
  }

  Output LineCommand::cancel ( const Context & )
  {
    Output output = Output::finish ( m_state.hasAnchorPoint (), true );
    output.addHistory ( HistoryEntry::info ( "LINE canceled" ) );
    return output;
  }

  Preview LineCommand::preview () const
  {
    if ( ! m_state.lastPoint.has_value () || ! m_state.previewPoint.has_value () ) return {};

    Preview preview;
    preview.lines.push_back ( PreviewLine{
        .start = *m_state.lastPoint,
        .end = *m_state.previewPoint,
        .color = { 1.0f, 0.72f, 0.24f, 0.95f },
    } );
    return preview;
  }

  std::string LineCommand::prompt () const
  {
    if ( ! m_state.hasAnchorPoint () ) return "Specify first point";
    if ( m_state.canClose () ) return "Specify next point, [Close], or press Enter to finish";
    return "Specify next point or press Enter to finish";
  }

  Output LineCommand::closeLine ( const Context &context )
  {
    Output output = Output::finish ( true, true );

    if ( ! samePoint ( *m_state.lastPoint, *m_state.firstPoint ) )
    {
      context.editor.addLine ( { *m_state.lastPoint, *m_state.firstPoint } );
      output.addHistory ( HistoryEntry::info ( "Close: " + formatPoint ( *m_state.firstPoint ) ) );
    }
    else
      output.addHistory ( HistoryEntry::info ( "Close" ) );

    output.addHistory ( HistoryEntry::info ( "LINE finished" ) );
    return output;
  }

  Output LineCommand::applyWorldPoint ( const Context &context, const glm::dvec2 &point,
                                        const std::string_view sourceText )
  {
    if ( ! m_state.hasAnchorPoint () ) return beginLine ( point, sourceText );
    return commitLine ( context, point, sourceText );
  }

  Output LineCommand::beginLine ( const glm::dvec2 &point, const std::string_view sourceText )
  {
    m_state.firstPoint = point;
    m_state.lastPoint = point;
    m_state.previewPoint = point;
    m_state.segmentCount = 0;

    Output output = Output::handledAndRepaint ( true );
    output.addHistory (
        HistoryEntry::info ( "First point: " + describePoint ( point, sourceText ) ) );
    return output;
  }

  Output LineCommand::commitLine ( const Context &context, const glm::dvec2 &point,
                                   const std::string_view sourceText )
  {
    context.editor.addLine ( { *m_state.lastPoint, point } );
    m_state.lastPoint = point;
    m_state.previewPoint = point;
    ++m_state.segmentCount;

    Output output = Output::handledAndRepaint ( true );
    output.addHistory (
        HistoryEntry::info ( "Next point: " + describePoint ( point, sourceText ) ) );
    return output;
  }

  std::string LineCommand::describePoint ( const glm::dvec2 &point,
                                           const std::string_view sourceText )
  {
    const std::string formattedPoint = formatPoint ( point );
    if ( sourceText.empty () ) return formattedPoint;
    return std::string ( sourceText ) + " -> " + formattedPoint;
  }

  std::optional<glm::dvec2> LineCommand::basePoint () const noexcept { return m_state.lastPoint; }
} // namespace Qadra::Command
