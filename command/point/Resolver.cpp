#include "Resolver.hpp"

#include <cassert>
#include <cmath>
#include <numbers>

namespace
{
  [[nodiscard]] Qadra::Command::ResolveResult success ( const glm::dvec2 &point )
  {
    Qadra::Command::ResolveResult result;
    result.ok = true;
    result.point = point;
    return result;
  }

  [[nodiscard]] glm::dvec2 polarToOffset ( const double distance, const double angleDegrees )
  {
    const double angleRadians = angleDegrees * std::numbers::pi_v<double> / 180.0;
    return { distance * std::cos ( angleRadians ), distance * std::sin ( angleRadians ) };
  }
} // namespace

namespace Qadra::Command
{
  ResolveResult PointResolver::resolve ( const PointExpression &expression,
                                         const ResolveContext &context ) const
  {
    const glm::dvec2 origin = expression.relative
                                  ? context.basePoint.value_or ( glm::dvec2{ 0.0, 0.0 } )
                                  : glm::dvec2{ 0.0, 0.0 };

    if ( const auto *cartesian = std::get_if<CartesianValue> ( &expression.value ) )
    {
      const glm::dvec2 point = expression.relative
                                   ? origin + glm::dvec2{ cartesian->x, cartesian->y }
                                   : glm::dvec2{ cartesian->x, cartesian->y };
      return success ( point );
    }

    const auto *polar = std::get_if<PolarValue> ( &expression.value );
    assert ( polar != nullptr );
    return success ( origin + polarToOffset ( polar->distance, polar->angleDegrees ) );
  }
} // namespace Qadra::Command
