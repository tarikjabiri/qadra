#ifndef QADRA_COMMAND_MANAGER_HPP
#define QADRA_COMMAND_MANAGER_HPP

#include "Context.hpp"
#include "Output.hpp"
#include "PointerEvent.hpp"
#include "Preview.hpp"
#include "Registry.hpp"
#include "View.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace Qadra::Command
{
  class SessionCommand;

  class Manager final
  {
  public:
    Manager ();
    ~Manager ();

    Manager ( const Manager & ) = delete;
    Manager &operator= ( const Manager & ) = delete;
    Manager ( Manager && ) = delete;
    Manager &operator= ( Manager && ) = delete;

    [[nodiscard]] Tool::ToolKind activeToolKind () const noexcept;

    [[nodiscard]] View view () const;

    [[nodiscard]] Preview preview () const;

    void setInput ( std::string text );

    void appendInput ( std::string_view text );

    void backspaceInput ();

    [[nodiscard]] Output start ( Tool::ToolKind kind, const Context &context );

    [[nodiscard]] Output submit ( const Context &context );

    [[nodiscard]] Output cancel ( const Context &context );

    [[nodiscard]] Output pointerPress ( const Context &context, const PointerEvent &event );

    [[nodiscard]] Output pointerMove ( const Context &context, const PointerEvent &event );

    [[nodiscard]] Output pointerRelease ( const Context &context, const PointerEvent &event );

  private:
    [[nodiscard]] static std::string_view trim ( std::string_view text ) noexcept;

    [[nodiscard]] static bool isCancelAlias ( std::string_view text ) noexcept;

    Output apply ( Output output, const Context &context );

    Registry m_registry;
    std::unique_ptr<SessionCommand> m_activeCommand;
    std::string m_input;
    std::vector<HistoryEntry> m_history;
  };
} // namespace Qadra::Command

#endif // QADRA_COMMAND_MANAGER_HPP
