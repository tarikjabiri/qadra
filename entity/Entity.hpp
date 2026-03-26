#ifndef QADRA_ENTITY_HPP
#define QADRA_ENTITY_HPP

#include "BoxAabb.hpp"
#include "EntityType.hpp"
#include "Handle.hpp"

#include <cstdint>

namespace Qadra::Entity
{
  class Entity
  {
  public:
    explicit Entity ( Core::Handle handle, EntityType type, std::uint32_t renderKey );
    virtual ~Entity () = default;

    Core::Handle handle () const { return m_handle; }

    EntityType type () const { return m_type; }

    std::uint32_t renderKey () const { return m_renderKey; }

    virtual Math::BoxAABB bbox () const = 0;

  private:
    Core::Handle m_handle;
    EntityType m_type;
    std::uint32_t m_renderKey;
  };
} // namespace Qadra::Entity

#endif // QADRA_ENTITY_HPP
