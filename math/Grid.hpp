#ifndef QADRA_MATH_GRID_HPP
#define QADRA_MATH_GRID_HPP

#include <glm/glm.hpp>

#include "Camera.hpp"

namespace Qadra::Math {
  class Grid {
  public:
    struct Line {
      glm::dvec2 start;
      glm::dvec2 end;
      bool major = false;
    };

    static double niceStep(double x);

    static std::vector<Line> compute(const Core::Camera &camera);
  };
}

#endif //QADRA_MATH_GRID_HPP
