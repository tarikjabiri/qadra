#include "Ellipse.hpp"

namespace Qadra::Entity
{
  Ellipse::Ellipse ( const Core::Handle handle, const std::uint32_t renderKey,
                     const EllipseRecord &record )
      : Entity ( handle, EntityType::Ellipse, renderKey ),
        m_curve ( record.center, record.majorDirection, record.majorRadius, record.minorRadius ),
        m_bbox ( m_curve.bbox () )
  {
  }

  Math::BoxAABB Ellipse::bbox () const { return m_bbox; }
} // namespace Qadra::Entity
