#ifndef QADRA_COMMAND_ELLIPSE_COMMAND_HPP
#define QADRA_COMMAND_ELLIPSE_COMMAND_HPP

#include "State.hpp"
#include "command/SessionCommand.hpp"
#include "command/point/Parser.hpp"
#include "command/point/Resolver.hpp"
#include "kernel/EllipseBuilder.hpp"

#include <optional>

namespace Qadra::Command
{
  class EllipseCommand final : public SessionCommand
  {
  public:
    EllipseCommand ();

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

    [[nodiscard]] Output setMajorAxisPoint ( const glm::dvec2 &point, std::string_view sourceText );

    [[nodiscard]] Output commitEllipse ( const Context &context, const glm::dvec2 &point,
                                         std::string_view sourceText );

    [[nodiscard]] std::optional<Math::Ellipse> buildEllipse ( const glm::dvec2 &axisPoint ) const;

    [[nodiscard]] static std::string describePoint ( const glm::dvec2 &point,
                                                     std::string_view sourceText );

    [[nodiscard]] std::optional<glm::dvec2> basePoint () const noexcept;

    EllipseState m_state;
    Kernel::EllipseBuilder m_ellipseBuilder;
    PointParser m_pointParser;
    PointResolver m_pointResolver;
  };
} // namespace Qadra::Command

#endif // QADRA_COMMAND_ELLIPSE_COMMAND_HPP
