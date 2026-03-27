#include "Manager.hpp"

#include "SessionCommand.hpp"

#include <cctype>
#include <iterator>
#include <utility>

namespace
{
  void mergeOutput ( Qadra::Command::Output &target, Qadra::Command::Output source )
  {
    target.handled = target.handled || source.handled;
    target.requestRepaint = target.requestRepaint || source.requestRepaint;
    target.viewChanged = target.viewChanged || source.viewChanged;
    target.finishCommand = target.finishCommand || source.finishCommand;

    target.historyEntries.insert ( target.historyEntries.end (),
                                   std::make_move_iterator ( source.historyEntries.begin () ),
                                   std::make_move_iterator ( source.historyEntries.end () ) );
  }
} // namespace

namespace Qadra::Command
{
  Manager::Manager () = default;
  Manager::~Manager () = default;

  Tool::ToolKind Manager::activeToolKind () const noexcept
  {
    return m_activeCommand ? m_activeCommand->kind () : Tool::ToolKind::None;
  }

  View Manager::view () const
  {
    return View{
        .activeToolKind = activeToolKind (),
        .prompt = m_activeCommand ? m_activeCommand->prompt () : std::string{ "Enter command" },
        .input = m_input,
        .history = m_history,
    };
  }

  Preview Manager::preview () const
  {
    return m_activeCommand ? m_activeCommand->preview () : Preview{};
  }

  void Manager::setInput ( std::string text ) { m_input = std::move ( text ); }

  void Manager::appendInput ( const std::string_view text ) { m_input.append ( text ); }

  void Manager::backspaceInput ()
  {
    if ( ! m_input.empty () ) m_input.pop_back ();
  }

  Output Manager::start ( const Tool::ToolKind kind, const Context &context )
  {
    if ( kind == Tool::ToolKind::None ) return cancel ( context );

    auto newCommand = m_registry.create ( kind );

    if ( ! newCommand )
    {
      Output output = Output::handledOnly ( true );
      output.addHistory ( HistoryEntry::error ( "Requested command is not available yet." ) );
      return apply ( std::move ( output ) );
    }

    Output output = Output::handledOnly ( true );

    if ( m_activeCommand )
    {
      mergeOutput ( output, m_activeCommand->cancel ( context ) );
      m_activeCommand.reset ();
    }

    m_input.clear ();
    m_activeCommand = std::move ( newCommand );
    mergeOutput ( output, m_activeCommand->start ( context ) );

    output.handled = true;
    output.viewChanged = true;
    output.finishCommand = false;
    return apply ( std::move ( output ) );
  }

  Output Manager::submit ( const Context &context )
  {
    const std::string submitted = std::string ( trim ( m_input ) );
    m_input.clear ();

    if ( submitted.empty () )
    {
      if ( ! m_activeCommand ) return Output::handledOnly ( true );

      Output output = m_activeCommand->finish ( context );
      output.handled = true;
      output.finishCommand = true;
      output.viewChanged = true;
      return apply ( std::move ( output ) );
    }

    if ( const auto commandKind = m_registry.resolveAlias ( submitted ) )
      return start ( *commandKind, context );

    if ( isCancelAlias ( submitted ) ) return cancel ( context );

    if ( ! m_activeCommand )
    {
      Output output = Output::handledOnly ( true );
      output.addHistory ( HistoryEntry::error ( "Unknown command." ) );
      return apply ( std::move ( output ) );
    }

    Output output = m_activeCommand->submit ( context, submitted );
    output.handled = true;
    output.viewChanged = true;
    return apply ( std::move ( output ) );
  }

  Output Manager::cancel ( const Context &context )
  {
    if ( ! m_activeCommand )
    {
      if ( m_input.empty () ) return Output::ignored ();

      m_input.clear ();
      return Output::handledOnly ( true );
    }

    m_input.clear ();

    Output output = m_activeCommand->cancel ( context );
    output.handled = true;
    output.finishCommand = true;
    output.viewChanged = true;
    return apply ( std::move ( output ) );
  }

  Output Manager::pointerPress ( const Context &context, const PointerEvent &event )
  {
    if ( ! m_activeCommand ) return Output::ignored ();
    return apply ( m_activeCommand->pointerPress ( context, event ) );
  }

  Output Manager::pointerMove ( const Context &context, const PointerEvent &event )
  {
    if ( ! m_activeCommand ) return Output::ignored ();
    return apply ( m_activeCommand->pointerMove ( context, event ) );
  }

  Output Manager::pointerRelease ( const Context &context, const PointerEvent &event )
  {
    if ( ! m_activeCommand ) return Output::ignored ();
    return apply ( m_activeCommand->pointerRelease ( context, event ) );
  }

  std::string_view Manager::trim ( std::string_view text ) noexcept
  {
    while ( ! text.empty () && std::isspace ( static_cast<unsigned char> ( text.front () ) ) )
      text.remove_prefix ( 1 );

    while ( ! text.empty () && std::isspace ( static_cast<unsigned char> ( text.back () ) ) )
      text.remove_suffix ( 1 );

    return text;
  }

  bool Manager::isCancelAlias ( const std::string_view text ) noexcept
  {
    const std::string_view trimmed = trim ( text );
    std::string normalized;
    normalized.reserve ( trimmed.size () );

    for ( const char ch : trimmed )
      normalized.push_back (
          static_cast<char> ( std::tolower ( static_cast<unsigned char> ( ch ) ) ) );

    return normalized == "cancel" || normalized == "esc" || normalized == "escape";
  }

  Output Manager::apply ( Output output )
  {
    if ( ! output.historyEntries.empty () )
    {
      m_history.insert ( m_history.end (),
                         std::make_move_iterator ( output.historyEntries.begin () ),
                         std::make_move_iterator ( output.historyEntries.end () ) );
      output.historyEntries.clear ();
    }

    if ( output.finishCommand ) m_activeCommand.reset ();

    return output;
  }
} // namespace Qadra::Command
