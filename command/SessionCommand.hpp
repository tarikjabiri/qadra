#ifndef QADRA_COMMAND_SESSION_COMMAND_HPP
#define QADRA_COMMAND_SESSION_COMMAND_HPP

#include "Context.hpp"
#include "Output.hpp"
#include "PointerEvent.hpp"
#include "Preview.hpp"
#include "tool/ToolKind.hpp"

#include <string>
#include <string_view>

namespace Qadra::Command
{
  class SessionCommand
  {
  public:
    explicit SessionCommand ( const Tool::ToolKind kind ) noexcept : m_kind ( kind ) { }

    virtual ~SessionCommand () = default;

    SessionCommand ( const SessionCommand & ) = delete;
    SessionCommand &operator= ( const SessionCommand & ) = delete;
    SessionCommand ( SessionCommand && ) = delete;
    SessionCommand &operator= ( SessionCommand && ) = delete;

    [[nodiscard]] Tool::ToolKind kind () const noexcept { return m_kind; }

    [[nodiscard]] virtual Output start ( const Context & ) { return Output::handledOnly ( true ); }

    [[nodiscard]] virtual Output submit ( const Context &, std::string_view text ) = 0;

    [[nodiscard]] virtual Output pointerPress ( const Context &, const PointerEvent & )
    {
      return Output::ignored ();
    }

    [[nodiscard]] virtual Output pointerMove ( const Context &, const PointerEvent & )
    {
      return Output::ignored ();
    }

    [[nodiscard]] virtual Output pointerRelease ( const Context &, const PointerEvent & )
    {
      return Output::ignored ();
    }

    [[nodiscard]] virtual Output finish ( const Context & ) = 0;

    [[nodiscard]] virtual Output cancel ( const Context & ) = 0;

    [[nodiscard]] virtual Preview preview () const { return {}; }

    [[nodiscard]] virtual std::string prompt () const = 0;

  private:
    Tool::ToolKind m_kind;
  };
} // namespace Qadra::Command

#endif // QADRA_COMMAND_SESSION_COMMAND_HPP
