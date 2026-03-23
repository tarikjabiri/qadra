#include "TextLayout.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace Qadra::Core {
  TextLayout measureTextLayout(Font &font, const std::string &text, const double height) {
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
    glm::dvec2 planeMinimum(
      std::numeric_limits<double>::infinity(),
      std::numeric_limits<double>::infinity()
    );
    glm::dvec2 planeMaximum(
      -std::numeric_limits<double>::infinity(),
      -std::numeric_limits<double>::infinity()
    );
    bool hasVisibleBounds = false;
    bool hasPlaneBounds = false;
    TextLayout layout;
    layout.glyphs.reserve(shapedGlyphs.size());

    for (const ShapedGlyph &shapedGlyph: shapedGlyphs) {
      const GlyphInfo &glyph = font.glyph(shapedGlyph.glyphId);
      const glm::dvec2 shapedOffset = shapedGlyph.offset * worldScale;
      const glm::dvec2 glyphOrigin = cursor + shapedOffset;
      const glm::dvec2 inkBoundsMinimum = glyphOrigin + glm::dvec2(glyph.inkBoundsMin) * worldScale;
      const glm::dvec2 inkBoundsMaximum = glyphOrigin + glm::dvec2(glyph.inkBoundsMax) * worldScale;
      const glm::dvec2 planeBoundsMinimum = glyphOrigin + glm::dvec2(glyph.planeBoundsMin) * worldScale;
      const glm::dvec2 planeBoundsMaximum = glyphOrigin + glm::dvec2(glyph.planeBoundsMax) * worldScale;
      const glm::dvec2 advance = shapedGlyph.advance * worldScale;

      if (inkBoundsMaximum.x > inkBoundsMinimum.x && inkBoundsMaximum.y > inkBoundsMinimum.y) {
        minimum.x = std::min(minimum.x, inkBoundsMinimum.x);
        minimum.y = std::min(minimum.y, inkBoundsMinimum.y);
        maximum.x = std::max(maximum.x, inkBoundsMaximum.x);
        maximum.y = std::max(maximum.y, inkBoundsMaximum.y);
        hasVisibleBounds = true;
      }

      if (planeBoundsMaximum.x > planeBoundsMinimum.x && planeBoundsMaximum.y > planeBoundsMinimum.y) {
        planeMinimum.x = std::min(planeMinimum.x, planeBoundsMinimum.x);
        planeMinimum.y = std::min(planeMinimum.y, planeBoundsMinimum.y);
        planeMaximum.x = std::max(planeMaximum.x, planeBoundsMaximum.x);
        planeMaximum.y = std::max(planeMaximum.y, planeBoundsMaximum.y);
        hasPlaneBounds = true;
      }

      layout.glyphs.push_back({
        .glyphId = shapedGlyph.glyphId,
        .cluster = shapedGlyph.cluster,
        .origin = glyphOrigin,
        .advance = advance,
        .uvMin = glyph.uvMin,
        .uvMax = glyph.uvMax,
        .inkBounds = {.minimum = inkBoundsMinimum, .maximum = inkBoundsMaximum},
        .planeBounds = {.minimum = planeBoundsMinimum, .maximum = planeBoundsMaximum},
      });

      cursor += advance;
    }

    layout.advance = cursor;

    if (!hasVisibleBounds) {
      layout.inkBounds = {
        .minimum = glm::dvec2(std::min(0.0, cursor.x), std::min(0.0, cursor.y)),
        .maximum = glm::dvec2(std::max(0.0, cursor.x), std::max(0.0, cursor.y))
      };
    } else {
      layout.inkBounds = {
        .minimum = minimum,
        .maximum = maximum
      };
    }

    if (!hasPlaneBounds) {
      layout.planeBounds = layout.inkBounds;
    } else {
      layout.planeBounds = {
        .minimum = planeMinimum,
        .maximum = planeMaximum
      };
    }

    return layout;
  }

  TextLayoutBounds measureTextLocalBounds(Font &font, const std::string &text, const double height) {
    return measureTextLayout(font, text, height).inkBounds;
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
