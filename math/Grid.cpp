#include "Grid.hpp"

#include <cmath>

namespace Qadra::Math {
  double Grid::niceStep(double x) {
    const double p = std::pow(10.0, std::floor(std::log10(x)));
    const double m = x / p;

    if (m <= 1.0) return 1.0 * p;
    if (m <= 2.0) return 2.0 * p;
    if (m <= 5.0) return 5.0 * p;
    return 10.0 * p;
  }

  std::vector<Grid::Line> Grid::compute(const Core::Camera &camera) {
    std::vector<Grid::Line> lines;

    const glm::dvec2 center = camera.position();
    const double zoom = camera.zoom();
    const double width = camera.width();
    const double height = camera.height();

    if (zoom <= 0.0) return lines;

    // Visible rectangle in world coordinates
    const double halfWidthWorld = width / (2.0 * zoom);
    const double halfHeightWorld = height / (2.0 * zoom);

    const double left   = center.x - halfWidthWorld;
    const double right  = center.x + halfWidthWorld;
    const double bottom = center.y - halfHeightWorld;
    const double top    = center.y + halfHeightWorld;

    // Choose grid spacing so lines are about 40 px apart
    constexpr double targetPixels = 10.0;
    const double desiredStep = targetPixels / zoom;
    const double step = niceStep(desiredStep);

    if (step <= 0.0) return lines;

    // Visible vertical lines
    const int ix0 = static_cast<int>(std::floor(left / step));
    const int ix1 = static_cast<int>(std::ceil(right / step));

    for (int i = ix0; i <= ix1; ++i) {
      const double x = i * step;
      const bool major = (i % 5) == 0;
      lines.push_back({
        glm::dvec2(x, bottom),
        glm::dvec2(x, top),
        major
      });
    }

    // Visible horizontal lines
    const int iy0 = static_cast<int>(std::floor(bottom / step));
    const int iy1 = static_cast<int>(std::ceil(top / step));

    for (int i = iy0; i <= iy1; ++i) {
      const double y = i * step;
      const bool major = (i % 5) == 0;
      lines.push_back({
        glm::dvec2(left, y),
        glm::dvec2(right, y),
        major
      });
    }


    return lines;
  }
}
