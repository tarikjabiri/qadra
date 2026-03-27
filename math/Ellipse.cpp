#include "Ellipse.hpp"

#include <cmath>

namespace
{
  constexpr double kDirectionEpsilon = 1e-9;
}

namespace Qadra::Math
{
  Ellipse::Ellipse ( const glm::dvec2 &center, const glm::dvec2 &majorDirection,
                     const double majorRadius, const double minorRadius )
      : m_center ( center ), m_majorRadius ( std::abs ( majorRadius ) ),
        m_minorRadius ( std::abs ( minorRadius ) )
  {
    const double directionLength = glm::length ( majorDirection );
    if ( directionLength <= kDirectionEpsilon ) m_majorDirection = { 1.0, 0.0 };
    else
      m_majorDirection = majorDirection / directionLength;
  }

  glm::dvec2 Ellipse::minorDirection () const noexcept
  {
    return { -m_majorDirection.y, m_majorDirection.x };
  }

  glm::dvec2 Ellipse::pointAtParameter ( const double parameter ) const
  {
    return m_center + m_majorDirection * ( std::cos ( parameter ) * m_majorRadius ) +
           minorDirection () * ( std::sin ( parameter ) * m_minorRadius );
  }

  glm::dvec2 Ellipse::majorAxisEndPoint () const
  {
    return m_center + m_majorDirection * m_majorRadius;
  }

  BoxAABB Ellipse::bbox () const
  {
    const glm::dvec2 minorDir = minorDirection ();
    const double extentX = std::sqrt ( std::pow ( m_majorRadius * m_majorDirection.x, 2.0 ) +
                                       std::pow ( m_minorRadius * minorDir.x, 2.0 ) );
    const double extentY = std::sqrt ( std::pow ( m_majorRadius * m_majorDirection.y, 2.0 ) +
                                       std::pow ( m_minorRadius * minorDir.y, 2.0 ) );

    return BoxAABB ( m_center - glm::dvec2 ( extentX, extentY ),
                     m_center + glm::dvec2 ( extentX, extentY ) );
  }
} // namespace Qadra::Math
