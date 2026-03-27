#ifndef QADRA_KERNEL_ELLIPSE_BUILDER_HPP
#define QADRA_KERNEL_ELLIPSE_BUILDER_HPP

#include "math/Ellipse.hpp"

#include <glm/glm.hpp>
#include <optional>

namespace Qadra::Kernel
{
  class EllipseBuilder
  {
  public:
    [[nodiscard]] std::optional<Math::Ellipse>
    fromCenterMajorEndpointAndAxisPoint ( const glm::dvec2 &center,
                                          const glm::dvec2 &majorAxisEndpoint,
                                          const glm::dvec2 &axisPoint ) const;
  };
} // namespace Qadra::Kernel

#endif // QADRA_KERNEL_ELLIPSE_BUILDER_HPP
