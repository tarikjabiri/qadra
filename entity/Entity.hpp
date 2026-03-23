#ifndef QADRA_ENTITY_HPP
#define QADRA_ENTITY_HPP

#include "BoxAabb.hpp"
#include "EntityType.hpp"
#include "Handle.hpp"

namespace Qadra::Entity
{
  class Entity
  {
  public:
    explicit Entity ( Core::Handle handle, EntityType type );
    virtual ~Entity () = default;

    Core::Handle handle () const { return m_handle; }

    EntityType type () const { return m_type; }

    virtual Math::BoxAABB bbox () const = 0;

  private:
    Core::Handle m_handle;
    EntityType m_type;
  };
} // namespace Qadra::Entity

#endif // QADRA_ENTITY_HPP
