#ifndef QADRA_MATH_CIRCLE_HPP
#define QADRA_MATH_CIRCLE_HPP

#include "BoxAabb.hpp"

#include <glm/glm.hpp>

namespace Qadra::Math
{
  class Circle
  {
  public:
    Circle ( const glm::dvec2 &center, double radius );

    [[nodiscard]] const glm::dvec2 &center () const noexcept { return m_center; }

    [[nodiscard]] double radius () const noexcept { return m_radius; }

    [[nodiscard]] BoxAABB bbox () const;

  private:
    glm::dvec2 m_center{ 0.0 };
    double m_radius = 0.0;
  };
} // namespace Qadra::Math

#endif // QADRA_MATH_CIRCLE_HPP
