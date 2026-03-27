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
  EllipseCommand::EllipseCommand () : SessionCommand ( Tool::ToolKind::Ellipse ) { }

  Output EllipseCommand::start ( const Context & )
  {
    Output output = Output::handledOnly ( true );
    output.addHistory ( HistoryEntry::command ( "ELLIPSE" ) );
    return output;
  }

  Output EllipseCommand::submit ( const Context &context, const std::string_view text )
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

  Output EllipseCommand::pointerPress ( const Context &context, const PointerEvent &event )
  {
    if ( event.button != PointerButton::Left ) return Output::ignored ();
    return applyWorldPoint ( context, event.worldPosition );
  }

  Output EllipseCommand::pointerMove ( const Context &, const PointerEvent &event )
  {
    if ( ! m_state.hasCenterPoint () ) return Output::ignored ();

    m_state.previewPoint = event.worldPosition;
    return Output::handledAndRepaint ();
  }

  Output EllipseCommand::finish ( const Context & )
  {
    Output output = Output::finish ( m_state.hasCenterPoint (), true );
    output.addHistory ( HistoryEntry::info ( "ELLIPSE finished" ) );
    return output;
  }

  Output EllipseCommand::cancel ( const Context & )
  {
    Output output = Output::finish ( m_state.hasCenterPoint (), true );
    output.addHistory ( HistoryEntry::info ( "ELLIPSE canceled" ) );
    return output;
  }

  Preview EllipseCommand::preview () const
  {
    Preview preview;

    if ( ! m_state.hasCenterPoint () || ! m_state.previewPoint.has_value () ) return preview;

    if ( ! m_state.hasMajorAxisPoint () )
    {
      preview.lines.push_back ( PreviewLine{
          .start = *m_state.centerPoint,
          .end = *m_state.previewPoint,
          .color = kPreviewColor,
      } );
      return preview;
    }

    const auto ellipse = buildEllipse ( *m_state.previewPoint );
    if ( ellipse.has_value () )
    {
      preview.ellipses.push_back ( PreviewEllipse{
          .center = ellipse->center (),
          .majorDirection = ellipse->majorDirection (),
          .majorRadius = ellipse->majorRadius (),
          .minorRadius = ellipse->minorRadius (),
          .color = kPreviewColor,
      } );
      return preview;
    }

    preview.lines.push_back ( PreviewLine{
        .start = *m_state.centerPoint,
        .end = *m_state.majorAxisPoint,
        .color = kPreviewColor,
    } );
    preview.lines.push_back ( PreviewLine{
        .start = *m_state.majorAxisPoint,
        .end = *m_state.previewPoint,
        .color = kPreviewColor,
    } );
    return preview;
  }

  std::string EllipseCommand::prompt () const
  {
    if ( ! m_state.hasCenterPoint () ) return "Specify ellipse center";
    if ( ! m_state.hasMajorAxisPoint () ) return "Specify endpoint of major axis";
    return "Specify distance to other axis";
  }

  Output EllipseCommand::applyWorldPoint ( const Context &context, const glm::dvec2 &point,
                                           const std::string_view sourceText )
  {
    if ( ! m_state.hasCenterPoint () ) return setCenterPoint ( point, sourceText );
    if ( ! m_state.hasMajorAxisPoint () ) return setMajorAxisPoint ( point, sourceText );
    return commitEllipse ( context, point, sourceText );
  }

  Output EllipseCommand::setCenterPoint ( const glm::dvec2 &point,
                                          const std::string_view sourceText )
  {
    m_state.centerPoint = point;
    m_state.previewPoint = point;

    Output output = Output::handledAndRepaint ( true );
    output.addHistory ( HistoryEntry::info ( "Center: " + describePoint ( point, sourceText ) ) );
    return output;
  }

  Output EllipseCommand::setMajorAxisPoint ( const glm::dvec2 &point,
                                             const std::string_view sourceText )
  {
    m_state.majorAxisPoint = point;
    m_state.previewPoint = point;

    Output output = Output::handledAndRepaint ( true );
    output.addHistory (
        HistoryEntry::info ( "Major axis endpoint: " + describePoint ( point, sourceText ) ) );
    return output;
  }

  Output EllipseCommand::commitEllipse ( const Context &context, const glm::dvec2 &point,
                                         const std::string_view sourceText )
  {
    const auto ellipse = buildEllipse ( point );

    if ( ! ellipse.has_value () )
    {
      m_state.previewPoint = point;

      Output output = Output::handledAndRepaint ( true );
      output.addHistory (
          HistoryEntry::error ( "Ellipse requires a valid major axis and minor radius." ) );
      return output;
    }

    context.document.addEllipse ( {
        .center = ellipse->center (),
        .majorDirection = ellipse->majorDirection (),
        .majorRadius = ellipse->majorRadius (),
        .minorRadius = ellipse->minorRadius (),
    } );

    Output output = Output::finish ( true, true );
    output.addHistory (
        HistoryEntry::info ( "Other axis point: " + describePoint ( point, sourceText ) ) );
    output.addHistory ( HistoryEntry::info ( "ELLIPSE finished" ) );
    return output;
  }

  std::optional<Math::Ellipse> EllipseCommand::buildEllipse ( const glm::dvec2 &axisPoint ) const
  {
    if ( ! m_state.centerPoint.has_value () || ! m_state.majorAxisPoint.has_value () )
      return std::nullopt;

    return m_ellipseBuilder.fromCenterMajorEndpointAndAxisPoint (
        *m_state.centerPoint, *m_state.majorAxisPoint, axisPoint );
  }

  std::string EllipseCommand::describePoint ( const glm::dvec2 &point,
                                              const std::string_view sourceText )
  {
    const std::string formattedPoint = formatPoint ( point );
    if ( sourceText.empty () ) return formattedPoint;
    return std::string ( sourceText ) + " -> " + formattedPoint;
  }

  std::optional<glm::dvec2> EllipseCommand::basePoint () const noexcept
  {
    if ( m_state.majorAxisPoint.has_value () ) return m_state.majorAxisPoint;
    return m_state.centerPoint;
  }
} // namespace Qadra::Command
