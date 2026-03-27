#ifndef QADRA_COMMAND_POINT_RESOLVER_HPP
#define QADRA_COMMAND_POINT_RESOLVER_HPP

#include "Expression.hpp"

#include <glm/glm.hpp>
#include <optional>
#include <string>

namespace Qadra::Command
{
  struct ResolveContext
  {
    std::optional<glm::dvec2> basePoint;
  };

  struct ResolveResult
  {
    bool ok = false;
    glm::dvec2 point{ 0.0 };
    std::string message;
  };

  class PointResolver final
  {
  public:
    [[nodiscard]] ResolveResult resolve ( const PointExpression &expression,
                                          const ResolveContext &context ) const;
  };
} // namespace Qadra::Command

#endif // QADRA_COMMAND_POINT_RESOLVER_HPP
