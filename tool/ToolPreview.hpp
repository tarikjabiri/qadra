#ifndef QADRA_TOOL_PREVIEW_HPP
#define QADRA_TOOL_PREVIEW_HPP

#include <glm/glm.hpp>
#include <vector>

namespace Qadra::Tool
{
  struct ToolPreviewLine
  {
    glm::dvec2 start{ 0.0 };
    glm::dvec2 end{ 0.0 };
    glm::vec4 color{ 1.0f };
  };

  struct ToolPreview
  {
    std::vector<ToolPreviewLine> lines;

    [[nodiscard]] bool empty () const noexcept { return lines.empty (); }
  };
} // namespace Qadra::Tool

#endif // QADRA_TOOL_PREVIEW_HPP
