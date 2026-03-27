#include "Circle.hpp"

#include <cmath>

namespace Qadra::Math
{
  Circle::Circle ( const glm::dvec2 &center, const double radius )
      : m_center ( center ), m_radius ( std::abs ( radius ) )
  {
  }

  BoxAABB Circle::bbox () const
  {
    return BoxAABB ( m_center - glm::dvec2 ( m_radius ), m_center + glm::dvec2 ( m_radius ) );
  }
} // namespace Qadra::Math
