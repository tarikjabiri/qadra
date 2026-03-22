#include "TextLayout.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace Qadra::Core {
  TextLayoutBounds measureTextLocalBounds(Font &font, const std::string &text, const double height) {
    const double fontHeight = font.ascender() - font.descender();
    if (text.empty() || height <= 0.0 || fontHeight <= 0.0) {
      return {};
    }

    const double worldScale = height / fontHeight;
    const std::vector<ShapedGlyph> shapedGlyphs = font.shape(text);
    glm::dvec2 cursor(0.0);

    glm::dvec2 minimum(
      std::numeric_limits<double>::infinity(),
      std::numeric_limits<double>::infinity()
    );
    glm::dvec2 maximum(
      -std::numeric_limits<double>::infinity(),
      -std::numeric_limits<double>::infinity()
    );
    bool hasVisibleBounds = false;

    for (const ShapedGlyph &shapedGlyph: shapedGlyphs) {
      const GlyphInfo &glyph = font.glyph(shapedGlyph.glyphId);
      const glm::dvec2 glyphBoundsMinimum = glm::dvec2(glyph.planeBoundsMin) * worldScale;
      const glm::dvec2 glyphBoundsMaximum = glm::dvec2(glyph.planeBoundsMax) * worldScale;
      const glm::dvec2 shapedOffset = shapedGlyph.offset * worldScale;
      const glm::dvec2 glyphBottomLeft = cursor + shapedOffset + glyphBoundsMinimum;
      const glm::dvec2 glyphTopRight = cursor + shapedOffset + glyphBoundsMaximum;

      if (glyphTopRight.x > glyphBottomLeft.x && glyphTopRight.y > glyphBottomLeft.y) {
        minimum.x = std::min(minimum.x, glyphBottomLeft.x);
        minimum.y = std::min(minimum.y, glyphBottomLeft.y);
        maximum.x = std::max(maximum.x, glyphTopRight.x);
        maximum.y = std::max(maximum.y, glyphTopRight.y);
        hasVisibleBounds = true;
      }

      cursor += shapedGlyph.advance * worldScale;
    }

    if (!hasVisibleBounds) {
      return {
        .minimum = glm::dvec2(std::min(0.0, cursor.x), std::min(0.0, cursor.y)),
        .maximum = glm::dvec2(std::max(0.0, cursor.x), std::max(0.0, cursor.y))
      };
    }

    return {
      .minimum = minimum,
      .maximum = maximum
    };
  }

  std::array<glm::dvec2, 4> orientedTextCorners(const glm::dvec2 &position,
                                                const TextLayoutBounds &localBounds,
                                                const double rotation) {
    const bool shouldRotate = std::abs(rotation) > 1e-12;
    const double sine = shouldRotate ? std::sin(rotation) : 0.0;
    const double cosine = shouldRotate ? std::cos(rotation) : 1.0;

    const auto rotateAroundOrigin = [&](const glm::dvec2 &point) {
      if (!shouldRotate) {
        return point;
      }

      return glm::dvec2(
        point.x * cosine - point.y * sine,
        point.x * sine + point.y * cosine
      );
    };

    return {
      position + rotateAroundOrigin(glm::dvec2(localBounds.minimum.x, localBounds.minimum.y)),
      position + rotateAroundOrigin(glm::dvec2(localBounds.maximum.x, localBounds.minimum.y)),
      position + rotateAroundOrigin(glm::dvec2(localBounds.maximum.x, localBounds.maximum.y)),
      position + rotateAroundOrigin(glm::dvec2(localBounds.minimum.x, localBounds.maximum.y))
    };
  }

  TextLayoutBounds axisAlignedBounds(const std::array<glm::dvec2, 4> &corners) {
    glm::dvec2 minimum(
      std::numeric_limits<double>::infinity(),
      std::numeric_limits<double>::infinity()
    );
    glm::dvec2 maximum(
      -std::numeric_limits<double>::infinity(),
      -std::numeric_limits<double>::infinity()
    );

    for (const glm::dvec2 &corner: corners) {
      minimum.x = std::min(minimum.x, corner.x);
      minimum.y = std::min(minimum.y, corner.y);
      maximum.x = std::max(maximum.x, corner.x);
      maximum.y = std::max(maximum.y, corner.y);
    }

    return {
      .minimum = minimum,
      .maximum = maximum
    };
  }
} // Qadra::Core
