#ifndef QADRA_TEXT_HPP
#define QADRA_TEXT_HPP

#include "BoxAabb.hpp"
#include "Entity.hpp"
#include "glm/glm.hpp"

#include <QString>

namespace Qadra::Entity
{
  struct TextRecord
  {
    glm::dvec2 position;
    QString text;
    double height;
    double rotation = 0.0;
  };

  class Text : public Entity
  {
  public:
    Text ( Core::Handle handle, const TextRecord &record, const Math::BoxAABB &bbox );

    const glm::dvec2 &position () const { return m_position; }

    const QString &text () const { return m_text; }

    double height () const { return m_height; }

    double rotation () const { return m_rotation; }

    Math::BoxAABB bbox () const override;

  private:
    glm::dvec2 m_position;
    QString m_text;
    double m_height;
    double m_rotation;
    Math::BoxAABB m_bbox;
  };
} // namespace Qadra::Entity

#endif // QADRA_TEXT_HPP
