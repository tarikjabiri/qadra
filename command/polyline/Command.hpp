#ifndef QADRA_COMMAND_POLYLINE_COMMAND_HPP
#define QADRA_COMMAND_POLYLINE_COMMAND_HPP

#include "State.hpp"
#include "command/SessionCommand.hpp"
#include "command/point/Parser.hpp"
#include "command/point/Resolver.hpp"
#include "kernel/ArcBuilder.hpp"

#include <optional>

namespace Qadra::Command
{
  class PolylineCommand final : public SessionCommand
  {
  public:
    PolylineCommand ();

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

    [[nodiscard]] Output beginPolyline ( const glm::dvec2 &point, std::string_view sourceText );

    [[nodiscard]] Output commitLineVertex ( const glm::dvec2 &point, std::string_view sourceText );

    [[nodiscard]] Output setArcThroughPoint ( const glm::dvec2 &point,
                                              std::string_view sourceText );

    [[nodiscard]] Output commitArcVertex ( const glm::dvec2 &point, std::string_view sourceText );

    [[nodiscard]] Output switchMode ( PolylineMode mode );

    [[nodiscard]] Output closePolyline ( const Context &context );

    [[nodiscard]] Output finishPolyline ( const Context &context, bool closed );

    [[nodiscard]] std::optional<Math::Arc> buildArcSegment ( const glm::dvec2 &endPoint ) const;

    void appendCommittedPreviewSegments ( Preview &preview ) const;

    [[nodiscard]] std::optional<glm::dvec2> basePoint () const noexcept;

    [[nodiscard]] static std::string describePoint ( const glm::dvec2 &point,
                                                     std::string_view sourceText );

    PolylineState m_state;
    Kernel::ArcBuilder m_arcBuilder;
    PointParser m_pointParser;
    PointResolver m_pointResolver;
  };
} // namespace Qadra::Command

#endif // QADRA_COMMAND_POLYLINE_COMMAND_HPP
