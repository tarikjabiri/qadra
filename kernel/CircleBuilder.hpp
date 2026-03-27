#ifndef QADRA_KERNEL_CIRCLE_BUILDER_HPP
#define QADRA_KERNEL_CIRCLE_BUILDER_HPP

#include "math/Circle.hpp"

#include <glm/glm.hpp>
#include <optional>

namespace Qadra::Kernel
{
  class CircleBuilder
  {
  public:
    [[nodiscard]] std::optional<Math::Circle>
    fromCenterAndRadiusPoint ( const glm::dvec2 &center, const glm::dvec2 &radiusPoint ) const;
  };
} // namespace Qadra::Kernel

#endif // QADRA_KERNEL_CIRCLE_BUILDER_HPP
