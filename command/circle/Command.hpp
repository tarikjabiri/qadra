#ifndef QADRA_COMMAND_CIRCLE_COMMAND_HPP
#define QADRA_COMMAND_CIRCLE_COMMAND_HPP

#include "State.hpp"
#include "command/SessionCommand.hpp"
#include "command/point/Parser.hpp"
#include "command/point/Resolver.hpp"
#include "kernel/CircleBuilder.hpp"

#include <optional>

namespace Qadra::Command
{
  class CircleCommand final : public SessionCommand
  {
  public:
    CircleCommand ();

    [[nodiscard]] Output start ( const Context &context ) override;

    [[nodiscard]] Output submit ( const Context &context, std::string_view text ) override;

    [[nodiscard]] Output pointerPress ( const Context &context,
                                        const PointerEvent &event ) override;

    [[nodiscard]] Output pointerMove ( const Context &context, const PointerEvent &event ) override;

    [[nodiscard]] Output finish ( const Context &context ) override;

    [[nodiscard]] Output cancel ( const Context &context ) override;

    [[nodiscard]] Preview preview () const override;

    [[nodiscard]] std::string prompt () const override;

  private:
    [[nodiscard]] Output applyWorldPoint ( const Context &context, const glm::dvec2 &point,
                                           std::string_view sourceText = {} );

    [[nodiscard]] Output setCenterPoint ( const glm::dvec2 &point, std::string_view sourceText );

    [[nodiscard]] Output commitCircle ( const Context &context, const glm::dvec2 &point,
                                        std::string_view sourceText );

    [[nodiscard]] std::optional<Math::Circle> buildCircle ( const glm::dvec2 &radiusPoint ) const;

    [[nodiscard]] static std::string describePoint ( const glm::dvec2 &point,
                                                     std::string_view sourceText );

    [[nodiscard]] std::optional<glm::dvec2> basePoint () const noexcept;

    CircleState m_state;
    Kernel::CircleBuilder m_circleBuilder;
    PointParser m_pointParser;
    PointResolver m_pointResolver;
  };
} // namespace Qadra::Command

#endif // QADRA_COMMAND_CIRCLE_COMMAND_HPP
