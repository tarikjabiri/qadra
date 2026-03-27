#ifndef QADRA_COMMAND_PREVIEW_HPP
#define QADRA_COMMAND_PREVIEW_HPP

#include <glm/glm.hpp>
#include <vector>

namespace Qadra::Command
{
  enum class PreviewLineStyle
  {
    Solid,
    Dashed,
  };

  struct PreviewLine
  {
    glm::dvec2 start{ 0.0 };
    glm::dvec2 end{ 0.0 };
    glm::vec4 color{ 1.0f };
    PreviewLineStyle style = PreviewLineStyle::Solid;
  };

  struct PreviewArc
  {
    glm::dvec2 center{ 0.0 };
    double radius = 0.0;
    double startAngle = 0.0;
    double sweepAngle = 0.0;
    glm::vec4 color{ 1.0f };
  };

  struct PreviewEllipse
  {
    glm::dvec2 center{ 0.0 };
    glm::dvec2 majorDirection{ 1.0, 0.0 };
    double majorRadius = 0.0;
    double minorRadius = 0.0;
    glm::vec4 color{ 1.0f };
  };

  struct Preview
  {
    std::vector<PreviewLine> lines;
    std::vector<PreviewArc> arcs;
    std::vector<PreviewEllipse> ellipses;

    [[nodiscard]] bool empty () const noexcept
    {
      return lines.empty () && arcs.empty () && ellipses.empty ();
    }
  };
} // namespace Qadra::Command

#endif // QADRA_COMMAND_PREVIEW_HPP
