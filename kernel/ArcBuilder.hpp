#ifndef QADRA_KERNEL_ARC_BUILDER_HPP
#define QADRA_KERNEL_ARC_BUILDER_HPP

#include "math/Arc.hpp"

#include <glm/glm.hpp>
#include <optional>

namespace Qadra::Kernel
{
  class ArcBuilder
  {
  public:
    [[nodiscard]] std::optional<Math::Arc> fromThreePoints ( const glm::dvec2 &start,
                                                             const glm::dvec2 &through,
                                                             const glm::dvec2 &end ) const;
  };
} // namespace Qadra::Kernel

#endif // QADRA_KERNEL_ARC_BUILDER_HPP
