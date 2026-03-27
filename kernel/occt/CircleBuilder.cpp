#include "kernel/CircleBuilder.hpp"

#include <GCE2d_MakeCircle.hxx>
#include <Geom2d_Circle.hxx>
#include <Standard_Failure.hxx>
#include <gp_Circ2d.hxx>
#include <gp_Pnt2d.hxx>

namespace
{
  constexpr double kRadiusEpsilon = 1e-9;

  [[nodiscard]] glm::dvec2 toGlm ( const gp_Pnt2d &point ) noexcept
  {
    return { point.X (), point.Y () };
  }
} // namespace

namespace Qadra::Kernel
{
  std::optional<Math::Circle>
  CircleBuilder::fromCenterAndRadiusPoint ( const glm::dvec2 &center,
                                            const glm::dvec2 &radiusPoint ) const
  {
    try
    {
      GCE2d_MakeCircle makeCircle{ gp_Pnt2d{ center.x, center.y },
                                   gp_Pnt2d{ radiusPoint.x, radiusPoint.y } };
      if ( ! makeCircle.IsDone () ) return std::nullopt;

      const Handle ( Geom2d_Circle ) circleGeometry = makeCircle.Value ();
      if ( circleGeometry.IsNull () ) return std::nullopt;

      const gp_Circ2d circle = circleGeometry->Circ2d ();
      if ( circle.Radius () <= kRadiusEpsilon ) return std::nullopt;

      return Math::Circle{ toGlm ( circle.Location () ), circle.Radius () };
    }
    catch ( const Standard_Failure & )
    {
      return std::nullopt;
    }
  }
} // namespace Qadra::Kernel
