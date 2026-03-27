#include "Arc.hpp"

namespace Qadra::Entity
{
  Arc::Arc ( const Core::Handle handle, const std::uint32_t renderKey, const ArcRecord &record )
      : Entity ( handle, EntityType::Arc, renderKey ),
        m_curve ( record.center, record.radius, record.startAngle, record.sweepAngle ),
        m_bbox ( m_curve.bbox () )
  {
  }

  Math::BoxAABB Arc::bbox () const { return m_bbox; }
} // namespace Qadra::Entity
