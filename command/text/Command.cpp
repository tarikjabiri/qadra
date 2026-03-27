#include "Command.hpp"

#include "cad/history/DocumentEditor.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace
{
  constexpr double kDefaultTextHeight = 50.0;
  constexpr std::size_t kPreviewTextLimit = 40;

  [[nodiscard]] std::string formatPoint ( const glm::dvec2 &point )
  {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision ( 3 ) << point.x << ", " << point.y;
    return stream.str ();
  }
} // namespace

namespace Qadra::Command
{
  TextCommand::TextCommand () : SessionCommand ( Tool::ToolKind::Text ) { }

  Output TextCommand::start ( const Context & )
  {
    Output output = Output::handledOnly ( true );
    output.addHistory ( HistoryEntry::command ( "TEXT" ) );
    return output;
  }

  Output TextCommand::submit ( const Context &context, const std::string_view text )
  {
    if ( m_state.hasInsertionPoint () ) return commitText ( context, text );

    const auto expression = m_pointParser.parse ( text );
    if ( ! expression.has_value () )
    {
      Output output = Output::handledOnly ( true );
      output.addHistory ( HistoryEntry::error ( "Specify text insertion point first." ) );
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

    return setInsertionPoint ( resolved.point, text );
  }

  Output TextCommand::pointerPress ( const Context &, const PointerEvent &event )
  {
    if ( event.button != PointerButton::Left ) return Output::ignored ();
    return setInsertionPoint ( event.worldPosition );
  }

  Output TextCommand::finish ( const Context & )
  {
    Output output = Output::finish ( false, true );
    output.addHistory (
        HistoryEntry::info ( m_state.hasInsertionPoint () ? "TEXT canceled" : "TEXT finished" ) );
    return output;
  }

  Output TextCommand::cancel ( const Context & )
  {
    Output output = Output::finish ( false, true );
    output.addHistory ( HistoryEntry::info ( "TEXT canceled" ) );
    return output;
  }

  std::string TextCommand::prompt () const
  {
    if ( ! m_state.hasInsertionPoint () ) return "Specify text insertion point";
    return "Enter text content";
  }

  Output TextCommand::setInsertionPoint ( const glm::dvec2 &point,
                                          const std::string_view sourceText )
  {
    m_state.insertionPoint = point;

    Output output = Output::handledOnly ( true );
    output.addHistory (
        HistoryEntry::info ( "Insertion point: " + describePoint ( point, sourceText ) ) );
    return output;
  }

  Output TextCommand::commitText ( const Context &context, const std::string_view text )
  {
    if ( ! context.hasFont () )
    {
      Output output = Output::handledOnly ( true );
      output.addHistory ( HistoryEntry::error ( "Text command requires an active font." ) );
      return output;
    }

    if ( text.empty () )
    {
      Output output = Output::handledOnly ( true );
      output.addHistory ( HistoryEntry::error ( "Text content cannot be empty." ) );
      return output;
    }

    const Core::Handle handle = context.editor.addText ( {
        .position = *m_state.insertionPoint,
        .text = QString::fromStdString ( std::string ( text ) ),
        .height = kDefaultTextHeight,
        .rotation = 0.0,
    } );

    if ( ! handle.isValid () )
    {
      Output output = Output::handledOnly ( true );
      output.addHistory ( HistoryEntry::error ( "Failed to create text entity." ) );
      return output;
    }

    Output output = Output::finish ( true, true );
    output.addHistory ( HistoryEntry::info ( "Content: " + summarizeText ( text ) ) );
    output.addHistory ( HistoryEntry::info ( "TEXT finished" ) );
    return output;
  }

  std::string TextCommand::describePoint ( const glm::dvec2 &point,
                                           const std::string_view sourceText )
  {
    const std::string formattedPoint = formatPoint ( point );
    if ( sourceText.empty () ) return formattedPoint;
    return std::string ( sourceText ) + " -> " + formattedPoint;
  }

  std::string TextCommand::summarizeText ( const std::string_view text )
  {
    const std::size_t previewLength = std::min ( text.size (), kPreviewTextLimit );
    std::string summary ( text.substr ( 0, previewLength ) );
    if ( text.size () > previewLength ) summary.append ( "..." );
    return "\"" + summary + "\"";
  }

  std::optional<glm::dvec2> TextCommand::basePoint () const noexcept
  {
    return m_state.insertionPoint;
  }
} // namespace Qadra::Command
