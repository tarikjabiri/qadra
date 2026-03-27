#ifndef QADRA_MATH_ELLIPSE_HPP
#define QADRA_MATH_ELLIPSE_HPP

#include "BoxAabb.hpp"

#include <glm/glm.hpp>

namespace Qadra::Math
{
  class Ellipse
  {
  public:
    Ellipse ( const glm::dvec2 &center, const glm::dvec2 &majorDirection, double majorRadius,
              double minorRadius );

    [[nodiscard]] const glm::dvec2 &center () const noexcept { return m_center; }

    [[nodiscard]] const glm::dvec2 &majorDirection () const noexcept { return m_majorDirection; }

    [[nodiscard]] glm::dvec2 minorDirection () const noexcept;

    [[nodiscard]] double majorRadius () const noexcept { return m_majorRadius; }

    [[nodiscard]] double minorRadius () const noexcept { return m_minorRadius; }

    [[nodiscard]] glm::dvec2 pointAtParameter ( double parameter ) const;

    [[nodiscard]] glm::dvec2 majorAxisEndPoint () const;

    [[nodiscard]] BoxAABB bbox () const;

  private:
    glm::dvec2 m_center{ 0.0 };
    glm::dvec2 m_majorDirection{ 1.0, 0.0 };
    double m_majorRadius = 0.0;
    double m_minorRadius = 0.0;
  };
} // namespace Qadra::Math

#endif // QADRA_MATH_ELLIPSE_HPP
