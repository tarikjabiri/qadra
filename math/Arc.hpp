#ifndef QADRA_MATH_ARC_HPP
#define QADRA_MATH_ARC_HPP

#include "BoxAabb.hpp"

#include <glm/glm.hpp>
#include <optional>

namespace Qadra::Math
{
  class Arc
  {
  public:
    Arc ( const glm::dvec2 &center, double radius, double startAngle, double sweepAngle );

    [[nodiscard]] static std::optional<Arc>
    fromThreePoints ( const glm::dvec2 &start, const glm::dvec2 &through, const glm::dvec2 &end );

    [[nodiscard]] const glm::dvec2 &center () const noexcept { return m_center; }

    [[nodiscard]] double radius () const noexcept { return m_radius; }

    [[nodiscard]] double startAngle () const noexcept { return m_startAngle; }

    [[nodiscard]] double sweepAngle () const noexcept { return m_sweepAngle; }

    [[nodiscard]] double endAngle () const noexcept;

    [[nodiscard]] glm::dvec2 pointAtAngle ( double angle ) const;

    [[nodiscard]] glm::dvec2 startPoint () const;

    [[nodiscard]] glm::dvec2 endPoint () const;

    [[nodiscard]] bool containsAngle ( double angle ) const noexcept;

    [[nodiscard]] BoxAABB bbox () const;

  private:
    glm::dvec2 m_center{ 0.0 };
    double m_radius = 0.0;
    double m_startAngle = 0.0;
    double m_sweepAngle = 0.0;
  };
} // namespace Qadra::Math

#endif // QADRA_MATH_ARC_HPP
