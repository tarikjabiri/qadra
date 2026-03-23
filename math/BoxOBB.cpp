#include "BoxOBB.hpp"

namespace Qadra::Math
{
  BoxOBB::BoxOBB () : m_center ( 0.0 ), m_extents ( 0.0 ) { }

  BoxOBB::BoxOBB ( const glm::dvec2 &center, const glm::dvec2 &extents, const double angle )
      : m_center ( center ), m_extents ( extents ), m_angle ( angle )
  {
  }

  std::array<glm::dvec2, 4> BoxOBB::corners () const
  {
    const double c = std::cos ( m_angle );
    const double s = std::sin ( m_angle );
    const glm::dvec2 axisX ( c, s );
    const glm::dvec2 axisY ( -s, c );

    const glm::dvec2 ex = axisX * m_extents.x;
    const glm::dvec2 ey = axisY * m_extents.y;

    return std::array{ m_center - ex - ey, m_center + ex - ey, m_center + ex + ey,
                       m_center - ex + ey };
  }

  BoxOBB::operator BoxAABB () const
  {
    const auto points = corners ();
    BoxAABB result;
    for ( const auto &p : points ) result.expand ( p );
    return result;
  }
} // namespace Qadra::Math
