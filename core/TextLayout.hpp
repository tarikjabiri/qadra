#ifndef QADRA_CORE_TEXTLAYOUT_HPP
#define QADRA_CORE_TEXTLAYOUT_HPP

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "Font.hpp"

namespace Qadra::Core {
  struct TextLayoutBounds {
    glm::dvec2 minimum{0.0};
    glm::dvec2 maximum{0.0};
  };

  struct TextLayoutGlyph {
    std::uint32_t glyphId{};
    std::uint32_t cluster{};
    glm::dvec2 origin{0.0};
    glm::dvec2 advance{0.0};
    glm::vec2 uvMin{0.0f};
    glm::vec2 uvMax{0.0f};
    TextLayoutBounds inkBounds;
    TextLayoutBounds planeBounds;
  };

  struct TextLayout {
    std::vector<TextLayoutGlyph> glyphs;
    TextLayoutBounds inkBounds;
    TextLayoutBounds planeBounds;
    glm::dvec2 advance{0.0};
  };

  [[nodiscard]] TextLayout measureTextLayout(Font &font, const std::string &text, double height);

  [[nodiscard]] TextLayoutBounds measureTextLocalBounds(Font &font, const std::string &text, double height);

  [[nodiscard]] std::array<glm::dvec2, 4> orientedTextCorners(const glm::dvec2 &position,
                                                              const TextLayoutBounds &localBounds,
                                                              double rotation);

  [[nodiscard]] TextLayoutBounds axisAlignedBounds(const std::array<glm::dvec2, 4> &corners);
} // Qadra::Core

#endif // QADRA_CORE_TEXTLAYOUT_HPP
