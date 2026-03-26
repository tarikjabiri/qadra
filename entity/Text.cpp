#include "Text.hpp"

namespace Qadra::Entity
{
  Text::Text ( const Core::Handle handle, const std::uint32_t renderKey, const TextRecord &record,
               const Core::TextLayout &layout, const Math::BoxAABB &bbox )
      : Entity ( handle, EntityType::Text, renderKey ), m_position ( record.position ),
        m_text ( record.text ), m_height ( record.height ), m_rotation ( record.rotation ),
        m_layout ( layout ), m_bbox ( bbox )
  {
  }

  Math::BoxAABB Text::bbox () const { return m_bbox; }
} // namespace Qadra::Entity
