#ifndef QADRA_ENTITY_CIRCLE_HPP
#define QADRA_ENTITY_CIRCLE_HPP

#include "Entity.hpp"
#include "math/Circle.hpp"

#include <cstdint>

namespace Qadra::Entity
{
  struct CircleRecord
  {
    glm::dvec2 center;
    double radius = 0.0;
  };

  class Circle : public Entity
  {
  public:
    Circle ( Core::Handle handle, std::uint32_t renderKey, const CircleRecord &record );

    [[nodiscard]] const Math::Circle &curve () const noexcept { return m_curve; }

    [[nodiscard]] Math::BoxAABB bbox () const override;

  private:
    Math::Circle m_curve;
    Math::BoxAABB m_bbox;
  };
} // namespace Qadra::Entity

#endif // QADRA_ENTITY_CIRCLE_HPP
