#ifndef QADRA_DOCUMENT_HPP
#define QADRA_DOCUMENT_HPP

#include "Entity.hpp"
#include "Handle.hpp"
#include "Line.hpp"

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

    [[nodiscard]] const QList<Core::Handle> &drawOrder () const { return m_drawOrder; }

  private:
    quint64 m_handleSeed = 0;
    std::unordered_map<Core::Handle, std::unique_ptr<Entity::Entity>> m_entities;
    QList<Core::Handle> m_drawOrder;
  };
} // namespace Qadra::Cad

#endif // QADRA_DOCUMENT_HPP
