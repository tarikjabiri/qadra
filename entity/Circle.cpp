#include "Circle.hpp"

namespace Qadra::Entity
{
  Circle::Circle ( const Core::Handle handle, const std::uint32_t renderKey,
                   const CircleRecord &record )
      : Entity ( handle, EntityType::Circle, renderKey ), m_curve ( record.center, record.radius ),
        m_bbox ( m_curve.bbox () )
  {
  }

  Math::BoxAABB Circle::bbox () const { return m_bbox; }
} // namespace Qadra::Entity
