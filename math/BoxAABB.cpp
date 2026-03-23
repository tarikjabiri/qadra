#include "BoxAabb.hpp"

namespace Qadra::Math
{
  BoxAABB::BoxAABB ()
      : m_min ( std::numeric_limits<double>::max () ),
        m_max ( -std::numeric_limits<double>::max () )
  {
  }

  BoxAABB::BoxAABB ( const glm::dvec2 &first, const glm::dvec2 &second )
      : m_min ( glm::min ( first, second ) ), m_max ( glm::max ( first, second ) )
  {
  }
} // namespace Qadra::Math
