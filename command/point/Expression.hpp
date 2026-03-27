#ifndef QADRA_COMMAND_POINT_EXPRESSION_HPP
#define QADRA_COMMAND_POINT_EXPRESSION_HPP

#include <variant>

namespace Qadra::Command
{
  struct CartesianValue
  {
    double x = 0.0;
    double y = 0.0;
  };

  struct PolarValue
  {
    double distance = 0.0;
    double angleDegrees = 0.0;
  };

  struct PointExpression
  {
    bool relative = false;
    std::variant<CartesianValue, PolarValue> value;
  };
} // namespace Qadra::Command

#endif // QADRA_COMMAND_POINT_EXPRESSION_HPP
