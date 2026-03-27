#ifndef QADRA_RENDER_ELLIPSE_INSTANCE_BUILDER_HPP
#define QADRA_RENDER_ELLIPSE_INSTANCE_BUILDER_HPP

#include "EllipsePass.hpp"
#include "math/Ellipse.hpp"

namespace Qadra::Render
{
  [[nodiscard]] EllipsePass::Instance buildEllipseInstance ( const Math::Ellipse &ellipse,
                                                             const glm::vec4 &color,
                                                             std::uint32_t renderKey );
} // namespace Qadra::Render

#endif // QADRA_RENDER_ELLIPSE_INSTANCE_BUILDER_HPP
