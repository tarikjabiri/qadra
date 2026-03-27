#ifndef QADRA_MATH_BULGE_HPP
#define QADRA_MATH_BULGE_HPP

#include "Arc.hpp"

#include <glm/glm.hpp>
#include <optional>

namespace Qadra::Math
{
  [[nodiscard]] std::optional<Arc> arcFromBulge ( const glm::dvec2 &start, const glm::dvec2 &end,
                                                  double bulge );

  [[nodiscard]] double bulgeFromSweepAngle ( double sweepAngle ) noexcept;

  [[nodiscard]] double bulgeFromArc ( const Arc &arc ) noexcept;
} // namespace Qadra::Math

#endif // QADRA_MATH_BULGE_HPP
