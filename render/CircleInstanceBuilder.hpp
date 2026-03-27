#ifndef QADRA_RENDER_CIRCLE_INSTANCE_BUILDER_HPP
#define QADRA_RENDER_CIRCLE_INSTANCE_BUILDER_HPP

#include "EllipsePass.hpp"
#include "math/Circle.hpp"

namespace Qadra::Render
{
  [[nodiscard]] EllipsePass::Instance buildCircleInstance ( const Math::Circle &circle,
                                                            const glm::vec4 &color,
                                                            std::uint32_t renderKey );
} // namespace Qadra::Render

#endif // QADRA_RENDER_CIRCLE_INSTANCE_BUILDER_HPP
