#include "EntitySnapshot.hpp"

namespace Qadra::Cad
{
  Entity::EntityType EntitySnapshot::type () const noexcept
  {
    return std::visit (
        [] ( const auto &payload ) -> Entity::EntityType
        {
          using Payload = std::decay_t<decltype ( payload )>;

          if constexpr ( std::is_same_v<Payload, Entity::LineRecord> )
            return Entity::EntityType::Line;
          if constexpr ( std::is_same_v<Payload, Entity::ArcRecord> )
            return Entity::EntityType::Arc;
          if constexpr ( std::is_same_v<Payload, Entity::CircleRecord> )
            return Entity::EntityType::Circle;
          if constexpr ( std::is_same_v<Payload, Entity::EllipseRecord> )
            return Entity::EntityType::Ellipse;
          if constexpr ( std::is_same_v<Payload, Entity::LWPolylineRecord> )
            return Entity::EntityType::LWPolyline;
          return Entity::EntityType::Text;
        },
        payload );
  }

  std::optional<EntitySnapshot> snapshotFromEntity ( const Entity::Entity &entity,
                                                     const std::size_t drawIndex )
  {
    EntitySnapshot snapshot{
        .handle = entity.handle (),
        .renderKey = entity.renderKey (),
        .drawIndex = drawIndex,
    };

    switch ( entity.type () )
    {
      case Entity::EntityType::Line:
      {
        const auto &line = static_cast<const Entity::Line &> ( entity );
        snapshot.payload = Entity::LineRecord{
            .start = line.start (),
            .end = line.end (),
        };
        return snapshot;
      }
      case Entity::EntityType::Arc:
      {
        const auto &arc = static_cast<const Entity::Arc &> ( entity );
        snapshot.payload = Entity::ArcRecord{
            .center = arc.curve ().center (),
            .radius = arc.curve ().radius (),
            .startAngle = arc.curve ().startAngle (),
            .sweepAngle = arc.curve ().sweepAngle (),
        };
        return snapshot;
      }
      case Entity::EntityType::Circle:
      {
        const auto &circle = static_cast<const Entity::Circle &> ( entity );
        snapshot.payload = Entity::CircleRecord{
            .center = circle.curve ().center (),
            .radius = circle.curve ().radius (),
        };
        return snapshot;
      }
      case Entity::EntityType::Ellipse:
      {
        const auto &ellipse = static_cast<const Entity::Ellipse &> ( entity );
        snapshot.payload = Entity::EllipseRecord{
            .center = ellipse.curve ().center (),
            .majorDirection = ellipse.curve ().majorDirection (),
            .majorRadius = ellipse.curve ().majorRadius (),
            .minorRadius = ellipse.curve ().minorRadius (),
        };
        return snapshot;
      }
      case Entity::EntityType::LWPolyline:
      {
        const auto &polyline = static_cast<const Entity::LWPolyline &> ( entity );
        snapshot.payload = Entity::LWPolylineRecord{
            .vertices = polyline.vertices (),
            .closed = polyline.closed (),
        };
        return snapshot;
      }
      case Entity::EntityType::Text:
      {
        const auto &text = static_cast<const Entity::Text &> ( entity );
        snapshot.payload = TextEntitySnapshot{
            .record =
                Entity::TextRecord{
                    .position = text.position (),
                    .text = text.text (),
                    .height = text.height (),
                    .rotation = text.rotation (),
                },
            .layout = text.layout (),
            .bbox = text.bbox (),
        };
        return snapshot;
      }
    }

    return std::nullopt;
  }
} // namespace Qadra::Cad
