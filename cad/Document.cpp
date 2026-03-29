#include "Document.hpp"

#include <algorithm>
#include <memory>
#include <utility>

namespace Qadra::Cad
{
  namespace
  {
    [[nodiscard]] std::unique_ptr<Entity::Entity> makeEntity ( const EntitySnapshot &snapshot )
    {
      switch ( snapshot.type () )
      {
        case Entity::EntityType::Line:
        {
          const auto &record = std::get<Entity::LineRecord> ( snapshot.payload );
          return std::make_unique<Entity::Line> ( snapshot.handle, snapshot.renderKey, record );
        }
        case Entity::EntityType::Arc:
        {
          const auto &record = std::get<Entity::ArcRecord> ( snapshot.payload );
          return std::make_unique<Entity::Arc> ( snapshot.handle, snapshot.renderKey, record );
        }
        case Entity::EntityType::Circle:
        {
          const auto &record = std::get<Entity::CircleRecord> ( snapshot.payload );
          return std::make_unique<Entity::Circle> ( snapshot.handle, snapshot.renderKey, record );
        }
        case Entity::EntityType::Ellipse:
        {
          const auto &record = std::get<Entity::EllipseRecord> ( snapshot.payload );
          return std::make_unique<Entity::Ellipse> ( snapshot.handle, snapshot.renderKey, record );
        }
        case Entity::EntityType::LWPolyline:
        {
          const auto &record = std::get<Entity::LWPolylineRecord> ( snapshot.payload );
          return std::make_unique<Entity::LWPolyline> ( snapshot.handle, snapshot.renderKey,
                                                        record );
        }
        case Entity::EntityType::Text:
        {
          const auto &record = std::get<TextEntitySnapshot> ( snapshot.payload );
          return std::make_unique<Entity::Text> ( snapshot.handle, snapshot.renderKey,
                                                  record.record, record.layout, record.bbox );
        }
      }

      return nullptr;
    }

  } // namespace

  Core::Handle Document::addLine ( const Entity::LineRecord &record )
  {
    const EntitySnapshot snapshot{
        .handle = next (),
        .renderKey = ++m_renderKeySeed,
        .drawIndex = static_cast<std::size_t> ( m_drawOrder.size () ),
        .payload = record,
    };

    [[maybe_unused]] const bool restored = restoreEntity ( snapshot );
    return snapshot.handle;
  }

  Core::Handle Document::addArc ( const Entity::ArcRecord &record )
  {
    const EntitySnapshot snapshot{
        .handle = next (),
        .renderKey = ++m_renderKeySeed,
        .drawIndex = static_cast<std::size_t> ( m_drawOrder.size () ),
        .payload = record,
    };

    [[maybe_unused]] const bool restored = restoreEntity ( snapshot );
    return snapshot.handle;
  }

  Core::Handle Document::addCircle ( const Entity::CircleRecord &record )
  {
    const EntitySnapshot snapshot{
        .handle = next (),
        .renderKey = ++m_renderKeySeed,
        .drawIndex = static_cast<std::size_t> ( m_drawOrder.size () ),
        .payload = record,
    };

    [[maybe_unused]] const bool restored = restoreEntity ( snapshot );
    return snapshot.handle;
  }

  Core::Handle Document::addEllipse ( const Entity::EllipseRecord &record )
  {
    const EntitySnapshot snapshot{
        .handle = next (),
        .renderKey = ++m_renderKeySeed,
        .drawIndex = static_cast<std::size_t> ( m_drawOrder.size () ),
        .payload = record,
    };

    [[maybe_unused]] const bool restored = restoreEntity ( snapshot );
    return snapshot.handle;
  }

  Core::Handle Document::addLWPolyline ( const Entity::LWPolylineRecord &record )
  {
    const EntitySnapshot snapshot{
        .handle = next (),
        .renderKey = ++m_renderKeySeed,
        .drawIndex = static_cast<std::size_t> ( m_drawOrder.size () ),
        .payload = record,
    };

    [[maybe_unused]] const bool restored = restoreEntity ( snapshot );
    return snapshot.handle;
  }

  Core::Handle Document::addText ( const Entity::TextRecord &record, Core::Font &font )
  {
    const auto layout = font.layout ( record.text );
    const auto bbox = computeTextBBox ( record, layout, font );
    const EntitySnapshot snapshot{
        .handle = next (),
        .renderKey = ++m_renderKeySeed,
        .drawIndex = static_cast<std::size_t> ( m_drawOrder.size () ),
        .payload =
            TextEntitySnapshot{
                .record = record,
                .layout = layout,
                .bbox = bbox,
            },
    };

    [[maybe_unused]] const bool restored = restoreEntity ( snapshot );
    return snapshot.handle;
  }

  Entity::Entity *Document::find ( const Core::Handle handle ) const
  {
    const auto it = m_entities.find ( handle );
    return it != m_entities.end () ? it->second.get () : nullptr;
  }

  std::optional<EntitySnapshot> Document::snapshotEntity ( const Core::Handle handle ) const
  {
    const Entity::Entity *entity = find ( handle );
    if ( ! entity ) return std::nullopt;

    const auto drawIndex = m_drawOrder.indexOf ( handle );
    if ( drawIndex < 0 ) return std::nullopt;

    return snapshotFromEntity ( *entity, static_cast<std::size_t> ( drawIndex ) );
  }

  std::optional<EntitySnapshot> Document::takeEntity ( const Core::Handle handle )
  {
    auto snapshot = snapshotEntity ( handle );
    if ( ! snapshot.has_value () ) return std::nullopt;

    const auto drawIndex = static_cast<qsizetype> ( snapshot->drawIndex );

    m_spatialIndex.remove ( handle );
    m_entities.erase ( handle );
    m_drawOrder.removeAt ( drawIndex );
    recordChange ( DocumentChange::Kind::Removed, handle );
    return snapshot;
  }

  bool Document::restoreEntity ( const EntitySnapshot &snapshot )
  {
    if ( ! snapshot.handle.isValid () ) return false;
    if ( m_entities.contains ( snapshot.handle ) ) return updateEntity ( snapshot );

    auto entity = makeEntity ( snapshot );
    if ( ! entity ) return false;

    const auto boundedIndex = static_cast<qsizetype> (
        std::min ( snapshot.drawIndex, static_cast<std::size_t> ( m_drawOrder.size () ) ) );

    m_spatialIndex.insert ( snapshot.handle, entity->bbox () );
    m_entities.emplace ( snapshot.handle, std::move ( entity ) );
    m_drawOrder.insert ( boundedIndex, snapshot.handle );
    m_handleSeed = std::max ( m_handleSeed, snapshot.handle.value () );
    m_renderKeySeed = std::max ( m_renderKeySeed, snapshot.renderKey );
    recordChange ( DocumentChange::Kind::Added, snapshot.handle );
    return true;
  }

  bool Document::updateEntity ( const EntitySnapshot &snapshot )
  {
    if ( ! snapshot.handle.isValid () ) return false;

    if ( m_drawOrder.indexOf ( snapshot.handle ) < 0 ) return false;

    auto entity = makeEntity ( snapshot );
    if ( ! entity ) return false;

    m_spatialIndex.update ( snapshot.handle, entity->bbox () );
    m_entities[snapshot.handle] = std::move ( entity );
    m_handleSeed = std::max ( m_handleSeed, snapshot.handle.value () );
    m_renderKeySeed = std::max ( m_renderKeySeed, snapshot.renderKey );
    recordChange ( DocumentChange::Kind::Modified, snapshot.handle );
    return true;
  }

  std::vector<DocumentChange> Document::drainChanges () const
  {
    auto changes = std::move ( m_pendingChanges );
    m_pendingChanges.clear ();
    return changes;
  }

  Math::BoxAABB Document::computeTextBBox ( const Entity::TextRecord &record,
                                            const Core::TextLayout &layout, Core::Font &font )
  {
    const double scale = record.height / font.unitsPerEm ();

    const double width = layout.advanceWidth * scale;
    const double ascender = font.ascender () * scale;
    const double descender = font.descender () * scale;

    const glm::dvec2 min = record.position + glm::dvec2 ( 0.0, descender );
    const glm::dvec2 max = record.position + glm::dvec2 ( width, ascender );

    if ( record.rotation == 0.0 ) return Math::BoxAABB ( min, max );

    // rotated — compute enclosing AABB of the four corners
    const double cos = std::cos ( record.rotation );
    const double sin = std::sin ( record.rotation );

    auto rotate = [&] ( const glm::dvec2 &offset ) -> glm::dvec2
    {
      return record.position +
             glm::dvec2 ( offset.x * cos - offset.y * sin, offset.x * sin + offset.y * cos );
    };

    const glm::dvec2 localMin = glm::dvec2 ( 0.0, descender );
    const glm::dvec2 localMax = glm::dvec2 ( width, ascender );

    Math::BoxAABB box;
    box.expand ( rotate ( localMin ) );
    box.expand ( rotate ( glm::dvec2 ( localMax.x, localMin.y ) ) );
    box.expand ( rotate ( glm::dvec2 ( localMin.x, localMax.y ) ) );
    box.expand ( rotate ( localMax ) );
    return box;
  }

  void Document::recordChange ( const DocumentChange::Kind kind, const Core::Handle handle ) const
  {
    m_pendingChanges.push_back ( { .kind = kind, .handle = handle } );
  }
} // namespace Qadra::Cad
