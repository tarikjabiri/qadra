#include "kernel/ArcBuilder.hpp"

#include <GCE2d_MakeArcOfCircle.hxx>
#include <Geom2d_Circle.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Standard_Failure.hxx>
#include <cmath>
#include <gp_Circ2d.hxx>
#include <gp_Pnt2d.hxx>
#include <numbers>

namespace
{
  constexpr double kTwoPi = 2.0 * std::numbers::pi_v<double>;
  constexpr double kAngleEpsilon = 1e-9;
  constexpr double kPointEpsilon = 1e-9;

  [[nodiscard]] double normalizeAngle ( double angle ) noexcept
  {
    angle = std::fmod ( angle, kTwoPi );
    if ( angle < 0.0 ) angle += kTwoPi;
    return angle;
  }

  [[nodiscard]] double ccwDelta ( const double from, const double to ) noexcept
  {
    return normalizeAngle ( to - from );
  }

  [[nodiscard]] glm::dvec2 toGlm ( const gp_Pnt2d &point ) noexcept
  {
    return { point.X (), point.Y () };
  }
} // namespace

namespace Qadra::Kernel
{
  std::optional<Math::Arc> ArcBuilder::fromThreePoints ( const glm::dvec2 &start,
                                                         const glm::dvec2 &through,
                                                         const glm::dvec2 &end ) const
  {
    try
    {
      GCE2d_MakeArcOfCircle makeArc{ gp_Pnt2d{ start.x, start.y }, gp_Pnt2d{ through.x, through.y },
                                     gp_Pnt2d{ end.x, end.y } };
      if ( ! makeArc.IsDone () ) return std::nullopt;

      const Handle ( Geom2d_TrimmedCurve ) trimmedCurve = makeArc.Value ();
      if ( trimmedCurve.IsNull () ) return std::nullopt;

      const Handle ( Geom2d_Circle ) circle =
          Handle ( Geom2d_Circle )::DownCast ( trimmedCurve->BasisCurve () );
      if ( circle.IsNull () ) return std::nullopt;

      const gp_Pnt2d centerPoint = circle->Circ2d ().Location ();
      const gp_Pnt2d startPoint = trimmedCurve->Value ( trimmedCurve->FirstParameter () );
      const gp_Pnt2d endPoint = trimmedCurve->Value ( trimmedCurve->LastParameter () );

      const glm::dvec2 center = toGlm ( centerPoint );
      const double radius = centerPoint.Distance ( startPoint );
      if ( radius <= kPointEpsilon ) return std::nullopt;

      const double startAngle =
          std::atan2 ( startPoint.Y () - center.y, startPoint.X () - center.x );
      const double throughAngle = std::atan2 ( through.y - center.y, through.x - center.x );
      const double endAngle = std::atan2 ( endPoint.Y () - center.y, endPoint.X () - center.x );

      const double ccwSweepToThrough = ccwDelta ( startAngle, throughAngle );
      const double ccwSweepToEnd = ccwDelta ( startAngle, endAngle );
      if ( ccwSweepToEnd <= kAngleEpsilon ) return std::nullopt;

      const double sweep =
          ( ccwSweepToThrough > kAngleEpsilon && ccwSweepToThrough < ccwSweepToEnd - kAngleEpsilon )
              ? ccwSweepToEnd
              : -( kTwoPi - ccwSweepToEnd );

      if ( std::abs ( sweep ) <= kAngleEpsilon ) return std::nullopt;

      return Math::Arc{ center, radius, startAngle, sweep };
    }
    catch ( const Standard_Failure & )
    {
      return std::nullopt;
    }
  }
} // namespace Qadra::Kernel
