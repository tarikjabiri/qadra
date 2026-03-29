#include "ArcInstanceBuilder.hpp"

#include <algorithm>
#include <cmath>

namespace
{
  [[nodiscard]] std::uint8_t packUnorm8 ( const float value )
  {
    const float clamped = std::clamp ( value, 0.0f, 1.0f );
    return static_cast<std::uint8_t> ( std::lround ( clamped * 255.0f ) );
  }
} // namespace

namespace Qadra::Render
{
  ArcPass::Instance buildArcInstance ( const Math::Arc &arc, const glm::vec4 &color,
                                       const std::uint32_t renderKey )
  {
    const Math::BoxAABB bbox = arc.bbox ();
    const glm::dvec2 center = arc.center ();

    return ArcPass::Instance{
        .centerWorld = center,
        .boundsMinLocal = glm::vec2 ( bbox.min () - center ),
        .boundsMaxLocal = glm::vec2 ( bbox.max () - center ),
        .radius = static_cast<float> ( arc.radius () ),
        .startAngle = static_cast<float> ( arc.startAngle () ),
        .sweepAngle = static_cast<float> ( arc.sweepAngle () ),
        .color =
            {
                packUnorm8 ( color.r ),
                packUnorm8 ( color.g ),
                packUnorm8 ( color.b ),
                packUnorm8 ( color.a ),
            },
        .renderKey = renderKey,
        .flags = 1u,
    };
  }
} // namespace Qadra::Render
