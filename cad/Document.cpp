#include "Document.hpp"

namespace Qadra::Cad
{
  Core::Handle Document::addLine ( const Entity::LineRecord &record )
  {
    auto handle = next ();
    m_entities.emplace ( handle,
                         std::make_unique<Entity::Line> ( handle, record.start, record.end ) );
    m_drawOrder.append ( handle );
    return handle;
  }

  Entity::Entity *Document::find ( const Core::Handle handle ) const
  {
    const auto it = m_entities.find ( handle );
    return it != m_entities.end () ? it->second.get () : nullptr;
  }
} // namespace Qadra::Cad
