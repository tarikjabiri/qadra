#ifndef QADRA_RENDER_ARC_INSTANCE_BUILDER_HPP
#define QADRA_RENDER_ARC_INSTANCE_BUILDER_HPP

#include "ArcPass.hpp"
#include "math/Arc.hpp"

namespace Qadra::Render
{
  [[nodiscard]] ArcPass::Instance buildArcInstance ( const Math::Arc &arc, const glm::vec4 &color,
                                                     std::uint32_t renderKey );
} // namespace Qadra::Render

#endif // QADRA_RENDER_ARC_INSTANCE_BUILDER_HPP
