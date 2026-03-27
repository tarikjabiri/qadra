#ifndef QADRA_DOCUMENT_HPP
#define QADRA_DOCUMENT_HPP

#include "Arc.hpp"
#include "DocumentChange.hpp"
#include "Ellipse.hpp"
#include "Entity.hpp"
#include "Font.hpp"
#include "Handle.hpp"
#include "Line.hpp"
#include "SpatialIndex.hpp"
#include "Text.hpp"

#include <QHash>
#include <vector>

namespace Qadra::Cad
{
  class Document
  {
  public:
    Document () = default;
    ~Document () = default;

    Core::Handle next () { return Core::Handle ( ++m_handleSeed ); }

    Core::Handle addLine ( const Entity::LineRecord &record );

    Core::Handle addArc ( const Entity::ArcRecord &record );

    Core::Handle addEllipse ( const Entity::EllipseRecord &record );

    Core::Handle addText ( const Entity::TextRecord &record, Core::Font &font );

    Entity::Entity *find ( Core::Handle handle ) const;

    void removeEntity ( Core::Handle handle );

    std::vector<DocumentChange> drainChanges () const;

    [[nodiscard]] const QList<Core::Handle> &drawOrder () const { return m_drawOrder; }

    const Spatial::SpatialIndex &spatialIndex () const { return m_spatialIndex; }

    std::size_t version () const { return m_version; }

    std::size_t dirtyFrom () const { return m_dirtyFrom; }

    std::uint32_t maxRenderKey () const { return m_renderKeySeed; }

    void resetDirty () const;

  private:
    static Math::BoxAABB computeTextBBox ( const Entity::TextRecord &record,
                                           const Core::TextLayout &layout, Core::Font &font );

    void recordChange ( DocumentChange::Kind kind, Core::Handle handle ) const;

    quint64 m_handleSeed = 0;
    std::uint32_t m_renderKeySeed = 0;
    std::unordered_map<Core::Handle, std::unique_ptr<Entity::Entity>> m_entities;
    QList<Core::Handle> m_drawOrder;
    Spatial::SpatialIndex m_spatialIndex;
    std::size_t m_version = 0;
    mutable std::size_t m_dirtyFrom = 0;
    mutable std::vector<DocumentChange> m_pendingChanges;
  };
} // namespace Qadra::Cad

#endif // QADRA_DOCUMENT_HPP
