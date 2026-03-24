#include "Document.hpp"

namespace Qadra::Cad
{
  Core::Handle Document::addLine ( const Entity::LineRecord &record )
  {
    auto handle = next ();
    auto entity = std::make_unique<Entity::Line> ( handle, record );
    m_spatialIndex.insert ( handle, entity->bbox () );
    m_entities.emplace ( handle, std::move ( entity ) );
    m_dirtyFrom = std::min ( m_dirtyFrom, static_cast<std::size_t> ( m_drawOrder.size () ) );
    m_drawOrder.append ( handle );
    ++m_version;
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
  }

  void Document::resetDirty () const { m_dirtyFrom = m_drawOrder.size (); }
} // namespace Qadra::Cad
