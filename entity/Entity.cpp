#include "Entity.hpp"

namespace Qadra::Entity
{
  Entity::Entity ( const Core::Handle handle, EntityType type )
      : m_handle ( handle ), m_type ( type )
  {
  }
} // namespace Qadra::Entity
