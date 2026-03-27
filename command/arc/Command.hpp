#ifndef QADRA_COMMAND_ARC_COMMAND_HPP
#define QADRA_COMMAND_ARC_COMMAND_HPP

#include "State.hpp"
#include "command/SessionCommand.hpp"
#include "command/point/Parser.hpp"
#include "command/point/Resolver.hpp"
#include "kernel/ArcBuilder.hpp"

#include <optional>

namespace Qadra::Command
{
  class ArcCommand final : public SessionCommand
  {
  public:
    ArcCommand ();

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

    [[nodiscard]] Output beginArc ( const glm::dvec2 &point, std::string_view sourceText );

    [[nodiscard]] Output setThroughPoint ( const glm::dvec2 &point, std::string_view sourceText );

    [[nodiscard]] Output commitArc ( const Context &context, const glm::dvec2 &point,
                                     std::string_view sourceText );

    [[nodiscard]] std::optional<Math::Arc> buildArc ( const glm::dvec2 &endPoint ) const;

    [[nodiscard]] static std::string describePoint ( const glm::dvec2 &point,
                                                     std::string_view sourceText );

    [[nodiscard]] std::optional<glm::dvec2> basePoint () const noexcept;

    ArcState m_state;
    Kernel::ArcBuilder m_arcBuilder;
    PointParser m_pointParser;
    PointResolver m_pointResolver;
  };
} // namespace Qadra::Command

#endif // QADRA_COMMAND_ARC_COMMAND_HPP
