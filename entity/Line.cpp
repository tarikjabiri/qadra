#include "entity/Line.hpp"

namespace Qadra::Entity
{
  Line::Line ( const Core::Handle handle, const std::uint32_t renderKey, const LineRecord &record )
      : Entity ( handle, EntityType::Line, renderKey ), m_start ( record.start ),
        m_end ( record.end )
  {
  }

  Math::BoxAABB Line::bbox () const { return Math::BoxAABB ( m_start, m_end ); }
} // namespace Qadra::Entity
