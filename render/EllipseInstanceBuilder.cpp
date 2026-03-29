#include "EllipseInstanceBuilder.hpp"

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
  EllipsePass::Instance buildEllipseInstance ( const Math::Ellipse &ellipse, const glm::vec4 &color,
                                               const std::uint32_t renderKey )
  {
    const Math::BoxAABB bbox = ellipse.bbox ();
    const glm::dvec2 center = ellipse.center ();

    return EllipsePass::Instance{
        .centerWorld = center,
        .boundsMinLocal = glm::vec2 ( bbox.min () - center ),
        .boundsMaxLocal = glm::vec2 ( bbox.max () - center ),
        .majorDirection = glm::vec2 ( ellipse.majorDirection () ),
        .majorRadius = static_cast<float> ( ellipse.majorRadius () ),
        .minorRadius = static_cast<float> ( ellipse.minorRadius () ),
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
