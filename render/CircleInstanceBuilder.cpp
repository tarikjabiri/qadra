#include "CircleInstanceBuilder.hpp"

#include "EllipseInstanceBuilder.hpp"
#include "math/Ellipse.hpp"

namespace Qadra::Render
{
  EllipsePass::Instance buildCircleInstance ( const Math::Circle &circle, const glm::vec4 &color,
                                              const std::uint32_t renderKey )
  {
    return buildEllipseInstance (
        Math::Ellipse ( circle.center (), { 1.0, 0.0 }, circle.radius (), circle.radius () ), color,
        renderKey );
  }
} // namespace Qadra::Render
