#ifndef QADRA_ENTITY_ARC_HPP
#define QADRA_ENTITY_ARC_HPP

#include "Entity.hpp"
#include "math/Arc.hpp"

#include <cstdint>

namespace Qadra::Entity
{
  struct ArcRecord
  {
    glm::dvec2 center;
    double radius = 0.0;
    double startAngle = 0.0;
    double sweepAngle = 0.0;
  };

  class Arc : public Entity
  {
  public:
    Arc ( Core::Handle handle, std::uint32_t renderKey, const ArcRecord &record );

    [[nodiscard]] const Math::Arc &curve () const noexcept { return m_curve; }

    [[nodiscard]] Math::BoxAABB bbox () const override;

  private:
    Math::Arc m_curve;
    Math::BoxAABB m_bbox;
  };
} // namespace Qadra::Entity

#endif // QADRA_ENTITY_ARC_HPP
