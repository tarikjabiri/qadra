#include "Entity.hpp"

namespace Qadra::Entity
{
  Entity::Entity ( const Core::Handle handle, EntityType type, const std::uint32_t renderKey )
      : m_handle ( handle ), m_type ( type ), m_renderKey ( renderKey )
  {
  }
} // namespace Qadra::Entity
