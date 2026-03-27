#include "kernel/EllipseBuilder.hpp"

#include <GCE2d_MakeEllipse.hxx>
#include <Geom2d_Ellipse.hxx>
#include <Standard_Failure.hxx>
#include <gp_Dir2d.hxx>
#include <gp_Elips2d.hxx>
#include <gp_Pnt2d.hxx>

namespace
{
  constexpr double kRadiusEpsilon = 1e-9;

  [[nodiscard]] glm::dvec2 toGlm ( const gp_Pnt2d &point ) noexcept
  {
    return { point.X (), point.Y () };
  }

  [[nodiscard]] glm::dvec2 toGlm ( const gp_Dir2d &direction ) noexcept
  {
    return { direction.X (), direction.Y () };
  }
} // namespace

namespace Qadra::Kernel
{
  std::optional<Math::Ellipse>
  EllipseBuilder::fromCenterMajorEndpointAndAxisPoint ( const glm::dvec2 &center,
                                                        const glm::dvec2 &majorAxisEndpoint,
                                                        const glm::dvec2 &axisPoint ) const
  {
    try
    {
      GCE2d_MakeEllipse makeEllipse{ gp_Pnt2d{ majorAxisEndpoint.x, majorAxisEndpoint.y },
                                     gp_Pnt2d{ axisPoint.x, axisPoint.y },
                                     gp_Pnt2d{ center.x, center.y } };
      if ( ! makeEllipse.IsDone () ) return std::nullopt;

      const Handle ( Geom2d_Ellipse ) ellipseGeometry = makeEllipse.Value ();
      if ( ellipseGeometry.IsNull () ) return std::nullopt;

      const gp_Elips2d ellipse = ellipseGeometry->Elips2d ();
      if ( ellipse.MajorRadius () <= kRadiusEpsilon || ellipse.MinorRadius () <= kRadiusEpsilon )
        return std::nullopt;

      return Math::Ellipse{ toGlm ( ellipse.Location () ), toGlm ( ellipse.Axis ().XDirection () ),
                            ellipse.MajorRadius (), ellipse.MinorRadius () };
    }
    catch ( const Standard_Failure & )
    {
      return std::nullopt;
    }
  }
} // namespace Qadra::Kernel
