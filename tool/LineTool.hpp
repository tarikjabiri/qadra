#ifndef QADRA_LINE_TOOL_HPP
#define QADRA_LINE_TOOL_HPP

#include "Tool.hpp"

#include <optional>

namespace Qadra::Tool
{
  class LineTool final : public Tool
  {
  public:
    LineTool ();

    void deactivate ( const ToolContext &context ) override;

    [[nodiscard]] ToolEventResult onPointerPress ( const ToolContext &context,
                                                   const ToolPointerEvent &event ) override;

    [[nodiscard]] ToolEventResult onPointerMove ( const ToolContext &context,
                                                  const ToolPointerEvent &event ) override;

    [[nodiscard]] ToolPreview preview ( const ToolContext &context ) const override;

    [[nodiscard]] ToolEventResult cancel ( const ToolContext &context ) override;

  private:
    [[nodiscard]] bool hasPendingStart () const noexcept;

    [[nodiscard]] ToolEventResult beginLine ( const ToolPointerEvent &event );

    [[nodiscard]] ToolEventResult commitLine ( const ToolContext &context,
                                               const ToolPointerEvent &event );

    [[nodiscard]] ToolEventResult resetState () noexcept;

    std::optional<glm::dvec2> m_startPoint;
    std::optional<glm::dvec2> m_currentPoint;
  };
} // namespace Qadra::Tool

#endif // QADRA_LINE_TOOL_HPP
