#ifndef QADRA_COMMAND_PREVIEW_HPP
#define QADRA_COMMAND_PREVIEW_HPP

#include <glm/glm.hpp>
#include <vector>

namespace Qadra::Command
{
  struct PreviewLine
  {
    glm::dvec2 start{ 0.0 };
    glm::dvec2 end{ 0.0 };
    glm::vec4 color{ 1.0f };
  };

  struct PreviewArc
  {
    glm::dvec2 center{ 0.0 };
    double radius = 0.0;
    double startAngle = 0.0;
    double sweepAngle = 0.0;
    glm::vec4 color{ 1.0f };
  };

  struct Preview
  {
    std::vector<PreviewLine> lines;
    std::vector<PreviewArc> arcs;

    [[nodiscard]] bool empty () const noexcept { return lines.empty () && arcs.empty (); }
  };
} // namespace Qadra::Command

#endif // QADRA_COMMAND_PREVIEW_HPP
