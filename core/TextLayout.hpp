#ifndef QADRA_CORE_TEXTLAYOUT_HPP
#define QADRA_CORE_TEXTLAYOUT_HPP

#include <array>
#include <string>

#include <glm/glm.hpp>

#include "Font.hpp"

namespace Qadra::Core {
  struct TextLayoutBounds {
    glm::dvec2 minimum{0.0};
    glm::dvec2 maximum{0.0};
  };

  [[nodiscard]] TextLayoutBounds measureTextLocalBounds(Font &font, const std::string &text, double height);

  [[nodiscard]] std::array<glm::dvec2, 4> orientedTextCorners(const glm::dvec2 &position,
                                                              const TextLayoutBounds &localBounds,
                                                              double rotation);

  [[nodiscard]] TextLayoutBounds axisAlignedBounds(const std::array<glm::dvec2, 4> &corners);
} // Qadra::Core

#endif // QADRA_CORE_TEXTLAYOUT_HPP
