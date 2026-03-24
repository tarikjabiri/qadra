#ifndef QADRA_DOCUMENT_HPP
#define QADRA_DOCUMENT_HPP

#include "Entity.hpp"
#include "Handle.hpp"
#include "Line.hpp"
#include "SpatialIndex.hpp"

#include <QHash>

namespace Qadra::Cad
{
  class Document
  {
  public:
    Document () = default;
    ~Document () = default;

    Core::Handle next () { return Core::Handle ( ++m_handleSeed ); }

    Core::Handle addLine ( const Entity::LineRecord &record );

    Entity::Entity *find ( Core::Handle handle ) const;

    void removeEntity ( Core::Handle handle );

    [[nodiscard]] const QList<Core::Handle> &drawOrder () const { return m_drawOrder; }

    const Spatial::SpatialIndex &spatialIndex () const { return m_spatialIndex; }

    std::size_t version () const { return m_version; }

    std::size_t dirtyFrom () const { return m_dirtyFrom; }

    void resetDirty () const;

  private:
    quint64 m_handleSeed = 0;
    std::unordered_map<Core::Handle, std::unique_ptr<Entity::Entity>> m_entities;
    QList<Core::Handle> m_drawOrder;
    Spatial::SpatialIndex m_spatialIndex;
    std::size_t m_version = 0;
    mutable std::size_t m_dirtyFrom = 0;
  };
} // namespace Qadra::Cad

#endif // QADRA_DOCUMENT_HPP
