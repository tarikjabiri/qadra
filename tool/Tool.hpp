#ifndef QADRA_TOOL_HPP
#define QADRA_TOOL_HPP

#include "ToolContext.hpp"
#include "ToolEventResult.hpp"
#include "ToolKind.hpp"
#include "ToolPointerEvent.hpp"

namespace Qadra::Tool
{
  class Tool
  {
  public:
    explicit Tool ( ToolKind kind ) noexcept : m_kind ( kind ) { }

    virtual ~Tool () = default;

    Tool ( const Tool & ) = delete;
    Tool &operator= ( const Tool & ) = delete;
    Tool ( Tool && ) = delete;
    Tool &operator= ( Tool && ) = delete;

    [[nodiscard]] ToolKind kind () const noexcept { return m_kind; }

    virtual void activate ( const ToolContext & ) { }

    virtual void deactivate ( const ToolContext & ) { }

    [[nodiscard]] virtual ToolEventResult onPointerPress ( const ToolContext &,
                                                           const ToolPointerEvent & )
    {
      return ToolEventResult::ignored ();
    }

    [[nodiscard]] virtual ToolEventResult onPointerMove ( const ToolContext &,
                                                          const ToolPointerEvent & )
    {
      return ToolEventResult::ignored ();
    }

    [[nodiscard]] virtual ToolEventResult onPointerRelease ( const ToolContext &,
                                                             const ToolPointerEvent & )
    {
      return ToolEventResult::ignored ();
    }

    [[nodiscard]] virtual ToolEventResult cancel ( const ToolContext &context ) = 0;

  private:
    ToolKind m_kind;
  };
} // namespace Qadra::Tool

#endif // QADRA_TOOL_HPP
