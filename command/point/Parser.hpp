#ifndef QADRA_COMMAND_POINT_PARSER_HPP
#define QADRA_COMMAND_POINT_PARSER_HPP

#include "Expression.hpp"

#include <optional>
#include <string_view>

namespace Qadra::Command
{
  class PointParser final
  {
  public:
    [[nodiscard]] std::optional<PointExpression> parse ( std::string_view text ) const;
  };
} // namespace Qadra::Command

#endif // QADRA_COMMAND_POINT_PARSER_HPP
