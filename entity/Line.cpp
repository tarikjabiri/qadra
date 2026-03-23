#include "entity/Line.hpp"

namespace Qadra::Entity
{
  Line::Line ( const Core::Handle handle, const glm::dvec2 &start, const glm::dvec2 &end )
      : Entity ( handle, EntityType::Line ), m_start ( start ), m_end ( end )
  {
  }

  Math::BoxAABB Line::bbox () const { return Math::BoxAABB ( m_start, m_end ); }
} // namespace Qadra::Entity
