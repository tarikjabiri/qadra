#ifndef QADRA_COMMAND_TEXT_COMMAND_HPP
#define QADRA_COMMAND_TEXT_COMMAND_HPP

#include "State.hpp"
#include "command/SessionCommand.hpp"
#include "command/point/Parser.hpp"
#include "command/point/Resolver.hpp"

#include <optional>

namespace Qadra::Command
{
  class TextCommand final : public SessionCommand
  {
  public:
    TextCommand ();

    [[nodiscard]] Output start ( const Context &context ) override;

    [[nodiscard]] Output submit ( const Context &context, std::string_view text ) override;

    [[nodiscard]] Output pointerPress ( const Context &context,
                                        const PointerEvent &event ) override;

    [[nodiscard]] Output finish ( const Context &context ) override;

    [[nodiscard]] Output cancel ( const Context &context ) override;

    [[nodiscard]] std::string prompt () const override;

  private:
    [[nodiscard]] Output setInsertionPoint ( const glm::dvec2 &point,
                                             std::string_view sourceText = {} );

    [[nodiscard]] Output commitText ( const Context &context, std::string_view text );

    [[nodiscard]] static std::string describePoint ( const glm::dvec2 &point,
                                                     std::string_view sourceText );

    [[nodiscard]] static std::string summarizeText ( std::string_view text );

    [[nodiscard]] std::optional<glm::dvec2> basePoint () const noexcept;

    TextState m_state;
    PointParser m_pointParser;
    PointResolver m_pointResolver;
  };
} // namespace Qadra::Command

#endif // QADRA_COMMAND_TEXT_COMMAND_HPP
