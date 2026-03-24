#include "Text.hpp"

namespace Qadra::Entity
{
  Text::Text ( const Core::Handle handle, const TextRecord &record, const Math::BoxAABB &bbox )
      : Entity ( handle, EntityType::Text ), m_position ( record.position ), m_text ( record.text ),
        m_height ( record.height ), m_rotation ( record.rotation ), m_bbox ( bbox )
  {
  }

  Math::BoxAABB Text::bbox () const { return m_bbox; }
} // namespace Qadra::Entity
