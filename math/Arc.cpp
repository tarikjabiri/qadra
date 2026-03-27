#include "Arc.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
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

  [[nodiscard]] bool nearlyEqual ( const glm::dvec2 &lhs, const glm::dvec2 &rhs ) noexcept
  {
    const glm::dvec2 delta = lhs - rhs;
    return glm::dot ( delta, delta ) <= kPointEpsilon * kPointEpsilon;
  }
} // namespace

namespace Qadra::Math
{
  Arc::Arc ( const glm::dvec2 &center, double radius, double startAngle, double sweepAngle )
      : m_center ( center ), m_radius ( radius ), m_startAngle ( normalizeAngle ( startAngle ) ),
        m_sweepAngle ( std::clamp ( sweepAngle, -kTwoPi, kTwoPi ) )
  {
  }

  std::optional<Arc> Arc::fromThreePoints ( const glm::dvec2 &start, const glm::dvec2 &through,
                                            const glm::dvec2 &end )
  {
    if ( nearlyEqual ( start, through ) || nearlyEqual ( through, end ) ||
         nearlyEqual ( start, end ) )
      return std::nullopt;

    const double ax = start.x;
    const double ay = start.y;
    const double bx = through.x;
    const double by = through.y;
    const double cx = end.x;
    const double cy = end.y;

    const double determinant = 2.0 * ( ax * ( by - cy ) + bx * ( cy - ay ) + cx * ( ay - by ) );
    if ( std::abs ( determinant ) <= kPointEpsilon ) return std::nullopt;

    const double aSquared = ax * ax + ay * ay;
    const double bSquared = bx * bx + by * by;
    const double cSquared = cx * cx + cy * cy;

    const glm::dvec2 center{
        ( aSquared * ( by - cy ) + bSquared * ( cy - ay ) + cSquared * ( ay - by ) ) / determinant,
        ( aSquared * ( cx - bx ) + bSquared * ( ax - cx ) + cSquared * ( bx - ax ) ) / determinant,
    };

    const double radius = glm::distance ( center, start );
    if ( radius <= kPointEpsilon ) return std::nullopt;

    const double startAngle = std::atan2 ( start.y - center.y, start.x - center.x );
    const double throughAngle = std::atan2 ( through.y - center.y, through.x - center.x );
    const double endAngle = std::atan2 ( end.y - center.y, end.x - center.x );

    const double ccwSweepToThrough = ccwDelta ( startAngle, throughAngle );
    const double ccwSweepToEnd = ccwDelta ( startAngle, endAngle );

    if ( ccwSweepToEnd <= kAngleEpsilon ) return std::nullopt;

    const double sweep =
        ( ccwSweepToThrough > kAngleEpsilon && ccwSweepToThrough < ccwSweepToEnd - kAngleEpsilon )
            ? ccwSweepToEnd
            : -( kTwoPi - ccwSweepToEnd );

    if ( std::abs ( sweep ) <= kAngleEpsilon ) return std::nullopt;

    return Arc{ center, radius, startAngle, sweep };
  }

  double Arc::endAngle () const noexcept { return normalizeAngle ( m_startAngle + m_sweepAngle ); }

  glm::dvec2 Arc::pointAtAngle ( const double angle ) const
  {
    return m_center + glm::dvec2{ std::cos ( angle ), std::sin ( angle ) } * m_radius;
  }

  glm::dvec2 Arc::startPoint () const { return pointAtAngle ( m_startAngle ); }

  glm::dvec2 Arc::endPoint () const { return pointAtAngle ( m_startAngle + m_sweepAngle ); }

  bool Arc::containsAngle ( const double angle ) const noexcept
  {
    if ( std::abs ( std::abs ( m_sweepAngle ) - kTwoPi ) <= kAngleEpsilon ) return true;

    const double normalizedAngle = normalizeAngle ( angle );
    if ( m_sweepAngle >= 0.0 )
      return ccwDelta ( m_startAngle, normalizedAngle ) <= m_sweepAngle + kAngleEpsilon;

    return ccwDelta ( normalizedAngle, m_startAngle ) <= -m_sweepAngle + kAngleEpsilon;
  }

  BoxAABB Arc::bbox () const
  {
    BoxAABB box;
    box.expand ( startPoint () );
    box.expand ( endPoint () );

    constexpr double cardinalAngles[] = {
        0.0,
        std::numbers::pi_v<double> * 0.5,
        std::numbers::pi_v<double>,
        std::numbers::pi_v<double> * 1.5,
    };

    for ( const double cardinalAngle : cardinalAngles )
    {
      if ( containsAngle ( cardinalAngle ) ) box.expand ( pointAtAngle ( cardinalAngle ) );
    }

    return box;
  }
} // namespace Qadra::Math
