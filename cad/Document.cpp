#include "Document.hpp"

namespace Qadra::Cad
{
  Core::Handle Document::addLine ( const Entity::LineRecord &record )
  {
    auto handle = next ();
    auto entity = std::make_unique<Entity::Line> ( handle, ++m_renderKeySeed, record );
    m_spatialIndex.insert ( handle, entity->bbox () );
    m_entities.emplace ( handle, std::move ( entity ) );
    m_dirtyFrom = std::min ( m_dirtyFrom, static_cast<std::size_t> ( m_drawOrder.size () ) );
    m_drawOrder.append ( handle );
    ++m_version;
    recordChange ( DocumentChange::Kind::Added, handle );
    return handle;
  }

  Core::Handle Document::addText ( const Entity::TextRecord &record, Core::Font &font )
  {
    auto handle = next ();

    const auto layout = font.layout ( record.text );
    const auto bbox = computeTextBBox ( record, layout, font );
    auto entity =
        std::make_unique<Entity::Text> ( handle, ++m_renderKeySeed, record, layout, bbox );

    m_spatialIndex.insert ( handle, entity->bbox () );
    m_entities.emplace ( handle, std::move ( entity ) );
    m_dirtyFrom = std::min ( m_dirtyFrom, static_cast<std::size_t> ( m_drawOrder.size () ) );
    m_drawOrder.append ( handle );
    ++m_version;
    recordChange ( DocumentChange::Kind::Added, handle );

    return handle;
  }

  Entity::Entity *Document::find ( const Core::Handle handle ) const
  {
    const auto it = m_entities.find ( handle );
    return it != m_entities.end () ? it->second.get () : nullptr;
  }

  void Document::removeEntity ( const Core::Handle handle )
  {
    if ( const auto idx = m_drawOrder.indexOf ( handle ); idx >= 0 )
      m_dirtyFrom = std::min ( m_dirtyFrom, static_cast<std::size_t> ( idx ) );

    m_spatialIndex.remove ( handle );
    m_entities.erase ( handle );
    m_drawOrder.removeOne ( handle );
    ++m_version;
    recordChange ( DocumentChange::Kind::Removed, handle );
  }

  std::vector<DocumentChange> Document::drainChanges () const
  {
    auto changes = std::move ( m_pendingChanges );
    m_pendingChanges.clear ();
    return changes;
  }

  void Document::resetDirty () const { m_dirtyFrom = m_drawOrder.size (); }

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
