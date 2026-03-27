#ifndef QADRA_COMMAND_LINE_COMMAND_HPP
#define QADRA_COMMAND_LINE_COMMAND_HPP

#include "State.hpp"
#include "command/SessionCommand.hpp"
#include "command/point/Parser.hpp"
#include "command/point/Resolver.hpp"

#include <optional>
#include <string_view>

namespace Qadra::Command
{
  class LineCommand final : public SessionCommand
  {
  public:
    LineCommand ();

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
    [[nodiscard]] Output closeLine ( const Context &context );

    [[nodiscard]] Output applyWorldPoint ( const Context &context, const glm::dvec2 &point,
                                           std::string_view sourceText = {} );

    [[nodiscard]] Output beginLine ( const glm::dvec2 &point, std::string_view sourceText );

    [[nodiscard]] Output commitLine ( const Context &context, const glm::dvec2 &point,
                                      std::string_view sourceText );

    [[nodiscard]] static std::string describePoint ( const glm::dvec2 &point,
                                                     std::string_view sourceText );

    [[nodiscard]] std::optional<glm::dvec2> basePoint () const noexcept;

    LineState m_state;
    PointParser m_pointParser;
    PointResolver m_pointResolver;
  };
} // namespace Qadra::Command

#endif // QADRA_COMMAND_LINE_COMMAND_HPP
