#ifndef QADRA_ENTITY_ELLIPSE_HPP
#define QADRA_ENTITY_ELLIPSE_HPP

#include "Entity.hpp"
#include "math/Ellipse.hpp"

#include <cstdint>

namespace Qadra::Entity
{
  struct EllipseRecord
  {
    glm::dvec2 center;
    glm::dvec2 majorDirection{ 1.0, 0.0 };
    double majorRadius = 0.0;
    double minorRadius = 0.0;
  };

  class Ellipse : public Entity
  {
  public:
    Ellipse ( Core::Handle handle, std::uint32_t renderKey, const EllipseRecord &record );

    [[nodiscard]] const Math::Ellipse &curve () const noexcept { return m_curve; }

    [[nodiscard]] Math::BoxAABB bbox () const override;

  private:
    Math::Ellipse m_curve;
    Math::BoxAABB m_bbox;
  };
} // namespace Qadra::Entity

#endif // QADRA_ENTITY_ELLIPSE_HPP
