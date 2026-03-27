#include "Command.hpp"

#include "cad/history/DocumentEditor.hpp"

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
  CircleCommand::CircleCommand () : SessionCommand ( Tool::ToolKind::Circle ) { }

  Output CircleCommand::start ( const Context & )
  {
    Output output = Output::handledOnly ( true );
    output.addHistory ( HistoryEntry::command ( "CIRCLE" ) );
    return output;
  }

  Output CircleCommand::submit ( const Context &context, const std::string_view text )
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

  Output CircleCommand::pointerPress ( const Context &context, const PointerEvent &event )
  {
    if ( event.button != PointerButton::Left ) return Output::ignored ();
    return applyWorldPoint ( context, event.worldPosition );
  }

  Output CircleCommand::pointerMove ( const Context &, const PointerEvent &event )
  {
    if ( ! m_state.hasCenterPoint () ) return Output::ignored ();

    m_state.previewPoint = event.worldPosition;
    return Output::handledAndRepaint ();
  }

  Output CircleCommand::finish ( const Context & )
  {
    Output output = Output::finish ( m_state.hasCenterPoint (), true );
    output.addHistory ( HistoryEntry::info ( "CIRCLE finished" ) );
    return output;
  }

  Output CircleCommand::cancel ( const Context & )
  {
    Output output = Output::finish ( m_state.hasCenterPoint (), true );
    output.addHistory ( HistoryEntry::info ( "CIRCLE canceled" ) );
    return output;
  }

  Preview CircleCommand::preview () const
  {
    Preview preview;
    if ( ! m_state.hasCenterPoint () || ! m_state.previewPoint.has_value () ) return preview;

    preview.lines.push_back ( PreviewLine{
        .start = *m_state.centerPoint,
        .end = *m_state.previewPoint,
        .color = kPreviewColor,
        .style = PreviewLineStyle::Dashed,
    } );

    const auto circle = buildCircle ( *m_state.previewPoint );
    if ( circle.has_value () )
    {
      preview.ellipses.push_back ( PreviewEllipse{
          .center = circle->center (),
          .majorDirection = { 1.0, 0.0 },
          .majorRadius = circle->radius (),
          .minorRadius = circle->radius (),
          .color = kPreviewColor,
      } );
    }

    return preview;
  }

  std::string CircleCommand::prompt () const
  {
    if ( ! m_state.hasCenterPoint () ) return "Specify circle center";
    return "Specify radius point";
  }

  Output CircleCommand::applyWorldPoint ( const Context &context, const glm::dvec2 &point,
                                          const std::string_view sourceText )
  {
    if ( ! m_state.hasCenterPoint () ) return setCenterPoint ( point, sourceText );
    return commitCircle ( context, point, sourceText );
  }

  Output CircleCommand::setCenterPoint ( const glm::dvec2 &point,
                                         const std::string_view sourceText )
  {
    m_state.centerPoint = point;
    m_state.previewPoint = point;

    Output output = Output::handledAndRepaint ( true );
    output.addHistory ( HistoryEntry::info ( "Center: " + describePoint ( point, sourceText ) ) );
    return output;
  }

  Output CircleCommand::commitCircle ( const Context &context, const glm::dvec2 &point,
                                       const std::string_view sourceText )
  {
    const auto circle = buildCircle ( point );

    if ( ! circle.has_value () )
    {
      m_state.previewPoint = point;

      Output output = Output::handledAndRepaint ( true );
      output.addHistory ( HistoryEntry::error ( "Circle requires a non-zero radius." ) );
      return output;
    }

    context.editor.addCircle ( {
        .center = circle->center (),
        .radius = circle->radius (),
    } );

    Output output = Output::finish ( true, true );
    output.addHistory (
        HistoryEntry::info ( "Radius point: " + describePoint ( point, sourceText ) ) );
    output.addHistory ( HistoryEntry::info ( "CIRCLE finished" ) );
    return output;
  }

  std::optional<Math::Circle> CircleCommand::buildCircle ( const glm::dvec2 &radiusPoint ) const
  {
    if ( ! m_state.centerPoint.has_value () ) return std::nullopt;
    return m_circleBuilder.fromCenterAndRadiusPoint ( *m_state.centerPoint, radiusPoint );
  }

  std::string CircleCommand::describePoint ( const glm::dvec2 &point,
                                             const std::string_view sourceText )
  {
    const std::string formattedPoint = formatPoint ( point );
    if ( sourceText.empty () ) return formattedPoint;
    return std::string ( sourceText ) + " -> " + formattedPoint;
  }

  std::optional<glm::dvec2> CircleCommand::basePoint () const noexcept
  {
    return m_state.centerPoint;
  }
} // namespace Qadra::Command
