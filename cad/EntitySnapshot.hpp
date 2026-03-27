#ifndef QADRA_CAD_ENTITY_SNAPSHOT_HPP
#define QADRA_CAD_ENTITY_SNAPSHOT_HPP

#include "Arc.hpp"
#include "BoxAabb.hpp"
#include "Circle.hpp"
#include "Ellipse.hpp"
#include "Entity.hpp"
#include "EntityType.hpp"
#include "Handle.hpp"
#include "LWPolyline.hpp"
#include "Line.hpp"
#include "Text.hpp"
#include "TextLayout.hpp"

#include <cstdint>
#include <optional>
#include <type_traits>
#include <variant>

namespace Qadra::Cad
{
  struct TextEntitySnapshot
  {
    Entity::TextRecord record;
    Core::TextLayout layout;
    Math::BoxAABB bbox;
  };

  using EntitySnapshotPayload =
      std::variant<Entity::LineRecord, Entity::ArcRecord, Entity::CircleRecord,
                   Entity::EllipseRecord, Entity::LWPolylineRecord, TextEntitySnapshot>;

  struct EntitySnapshot
  {
    Core::Handle handle;
    std::uint32_t renderKey = 0;
    std::size_t drawIndex = 0;
    EntitySnapshotPayload payload;

    [[nodiscard]] Entity::EntityType type () const noexcept;
  };

  [[nodiscard]] std::optional<EntitySnapshot> snapshotFromEntity ( const Entity::Entity &entity,
                                                                   std::size_t drawIndex );
} // namespace Qadra::Cad

#endif // QADRA_CAD_ENTITY_SNAPSHOT_HPP
