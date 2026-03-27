#include "Bulge.hpp"

#include <cmath>

namespace
{
  constexpr double kBulgeEpsilon = 1e-9;
}

namespace Qadra::Math
{
  std::optional<Arc> arcFromBulge ( const glm::dvec2 &start, const glm::dvec2 &end,
                                    const double bulge )
  {
    if ( std::abs ( bulge ) <= kBulgeEpsilon ) return std::nullopt;

    const glm::dvec2 chord = end - start;
    const double chordLength = glm::length ( chord );
    if ( chordLength <= kBulgeEpsilon ) return std::nullopt;

    const double sweepAngle = 4.0 * std::atan ( bulge );
    if ( std::abs ( sweepAngle ) <= kBulgeEpsilon ) return std::nullopt;

    const double radius = chordLength * ( 1.0 + bulge * bulge ) / ( 4.0 * std::abs ( bulge ) );
    const glm::dvec2 direction = chord / chordLength;
    const glm::dvec2 leftNormal{ -direction.y, direction.x };
    const double midpointToCenter = chordLength * ( 1.0 - bulge * bulge ) / ( 4.0 * bulge );
    const glm::dvec2 center = ( start + end ) * 0.5 + leftNormal * midpointToCenter;
    const double startAngle = std::atan2 ( start.y - center.y, start.x - center.x );

    return Arc{ center, radius, startAngle, sweepAngle };
  }

  double bulgeFromSweepAngle ( const double sweepAngle ) noexcept
  {
    return std::tan ( sweepAngle * 0.25 );
  }

  double bulgeFromArc ( const Arc &arc ) noexcept
  {
    return bulgeFromSweepAngle ( arc.sweepAngle () );
  }
} // namespace Qadra::Math
