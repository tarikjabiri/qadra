#include "InlineTextEditLayout.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace {
  struct InlineTextEditStyle {
    double framePaddingXPixels{10.0};
    double framePaddingYPixels{6.0};
    double frameBorderInsetPixels{1.4};
    double frameContentInsetPixels{3.2};
    double minimumCaretWidthPixels{1.0};
    double caretWidthFactor{0.012};
    double minimumCaretCapWidthPixels{4.0};
    double caretCapWidthFactor{0.18};
  };

  constexpr InlineTextEditStyle inlineTextEditStyle{};

  [[nodiscard]] Qadra::Core::TextLayoutBounds expandBounds(const Qadra::Core::TextLayoutBounds &bounds,
                                                           const double paddingX,
                                                           const double paddingY) {
    return {
      .minimum = glm::dvec2(bounds.minimum.x - paddingX, bounds.minimum.y - paddingY),
      .maximum = glm::dvec2(bounds.maximum.x + paddingX, bounds.maximum.y + paddingY)
    };
  }

  [[nodiscard]] Qadra::Core::TextLayoutBounds insetBounds(const Qadra::Core::TextLayoutBounds &bounds,
                                                          const double insetX,
                                                          const double insetY) {
    const double centerX = 0.5 * (bounds.minimum.x + bounds.maximum.x);
    const double centerY = 0.5 * (bounds.minimum.y + bounds.maximum.y);
    const double halfWidth = std::max(0.0, 0.5 * (bounds.maximum.x - bounds.minimum.x) - insetX);
    const double halfHeight = std::max(0.0, 0.5 * (bounds.maximum.y - bounds.minimum.y) - insetY);

    return {
      .minimum = glm::dvec2(centerX - halfWidth, centerY - halfHeight),
      .maximum = glm::dvec2(centerX + halfWidth, centerY + halfHeight)
    };
  }

  struct AdjacentGlyphEdge {
    bool exists{false};
    double xMin{};
    double xMax{};
  };

  [[nodiscard]] bool hasVisibleInkBounds(const Qadra::Core::TextLayoutGlyph &glyph) {
    return glyph.inkBounds.maximum.x > glyph.inkBounds.minimum.x &&
           glyph.inkBounds.maximum.y > glyph.inkBounds.minimum.y;
  }

  [[nodiscard]] AdjacentGlyphEdge glyphEdge(const Qadra::Core::TextLayoutGlyph &glyph) {
    const bool hasVisibleInk = hasVisibleInkBounds(glyph);
    return {
      .exists = true,
      .xMin = hasVisibleInk ? glyph.inkBounds.minimum.x : glyph.origin.x,
      .xMax = hasVisibleInk ? glyph.inkBounds.maximum.x : glyph.origin.x + glyph.advance.x
    };
  }

  [[nodiscard]] AdjacentGlyphEdge firstGlyphEdgeAtOrAfter(const Qadra::Core::TextLayout &textLayout,
                                                          const int byteOffset) {
    for (const Qadra::Core::TextLayoutGlyph &glyph: textLayout.glyphs) {
      if (static_cast<int>(glyph.cluster) >= byteOffset) {
        return glyphEdge(glyph);
      }
    }

    return {};
  }

  [[nodiscard]] AdjacentGlyphEdge lastGlyphEdgeBefore(const Qadra::Core::TextLayout &textLayout,
                                                      const int byteOffset) {
    AdjacentGlyphEdge edge;
    for (const Qadra::Core::TextLayoutGlyph &glyph: textLayout.glyphs) {
      if (static_cast<int>(glyph.cluster) < byteOffset) {
        edge = glyphEdge(glyph);
      }
    }

    return edge;
  }

  [[nodiscard]] double boundaryX(const Qadra::Core::TextLayout &textLayout, const int byteOffset) {
    const AdjacentGlyphEdge leftGlyphEdge = lastGlyphEdgeBefore(textLayout, byteOffset);
    if (leftGlyphEdge.exists) {
      return leftGlyphEdge.xMax;
    }

    const AdjacentGlyphEdge rightGlyphEdge = firstGlyphEdgeAtOrAfter(textLayout, byteOffset);
    if (rightGlyphEdge.exists) {
      return rightGlyphEdge.xMin;
    }

    return 0.0;
  }

  [[nodiscard]] Qadra::Core::TextLayoutBounds referenceLineBounds(Qadra::Core::Font &font, const double heightWorld) {
    const double fontHeight = font.ascender() - font.descender();
    if (fontHeight <= 0.0) {
      return {};
    }

    const double worldScale = heightWorld / fontHeight;
    return {
      .minimum = glm::dvec2(0.0, font.maximumGlyphBoundsMinimumY() * worldScale),
      .maximum = glm::dvec2(0.0, font.maximumGlyphBoundsMaximumY() * worldScale)
    };
  }
}

namespace Qadra::Core {
  TextLayoutBounds defaultInlineEditingLineBounds(Font &font, const double heightWorld) {
    return referenceLineBounds(font, heightWorld);
  }

  InlineTextEditLayout buildInlineTextEditLayout(const TextLayout &textLayout,
                                                 const double zoom,
                                                 const TextLayoutBounds &stableLineBoundsLocal,
                                                 const int cursorByteOffset,
                                                 const int selectionStartByteOffset,
                                                 const int selectionEndByteOffset) {
    const double safeZoom = std::max(zoom, 1e-6);
    const TextLayoutBounds lineBounds = stableLineBoundsLocal;

    TextLayoutBounds textBounds = textLayout.glyphs.empty()
      ? TextLayoutBounds{
          .minimum = glm::dvec2(0.0, lineBounds.minimum.y),
          .maximum = glm::dvec2(textLayout.advance.x, lineBounds.maximum.y)
        }
      : textLayout.inkBounds;

    if ((textBounds.maximum.y - textBounds.minimum.y) <= 1e-9) {
      textBounds.minimum.y = lineBounds.minimum.y;
      textBounds.maximum.y = lineBounds.maximum.y;
    }

    const double lineHeight = std::max(0.0, lineBounds.maximum.y - lineBounds.minimum.y);
    TextLayoutBounds contentBounds{
      .minimum = glm::dvec2(std::min(0.0, textBounds.minimum.x), lineBounds.minimum.y),
      .maximum = glm::dvec2(std::max(textBounds.maximum.x, 0.0), lineBounds.maximum.y)
    };

    const TextLayoutBounds frameBounds = expandBounds(
      contentBounds,
      inlineTextEditStyle.framePaddingXPixels / safeZoom,
      inlineTextEditStyle.framePaddingYPixels / safeZoom
    );
    const TextLayoutBounds frameInnerBounds = insetBounds(
      frameBounds,
      inlineTextEditStyle.frameBorderInsetPixels / safeZoom,
      inlineTextEditStyle.frameBorderInsetPixels / safeZoom
    );
    const TextLayoutBounds frameContentBounds = insetBounds(
      frameBounds,
      inlineTextEditStyle.frameContentInsetPixels / safeZoom,
      inlineTextEditStyle.frameContentInsetPixels / safeZoom
    );

    const double caretWidth = std::max(
      lineHeight * inlineTextEditStyle.caretWidthFactor,
      inlineTextEditStyle.minimumCaretWidthPixels / safeZoom
    );
    const double halfCaretWidth = caretWidth * 0.5;

    double caretCenterX = 0.0;
    const AdjacentGlyphEdge leftGlyphEdge = lastGlyphEdgeBefore(textLayout, cursorByteOffset);
    const AdjacentGlyphEdge rightGlyphEdge = firstGlyphEdgeAtOrAfter(textLayout, cursorByteOffset);
    if (leftGlyphEdge.exists) {
      caretCenterX = leftGlyphEdge.xMax + halfCaretWidth;
    } else if (rightGlyphEdge.exists) {
      caretCenterX = rightGlyphEdge.xMin - halfCaretWidth;
    }

    const double capWidth = std::max(
      lineHeight * inlineTextEditStyle.caretCapWidthFactor,
      inlineTextEditStyle.minimumCaretCapWidthPixels / safeZoom
    );
    const double halfCapWidth = 0.5 * capWidth;

    InlineTextEditLayout layout{
      .stableLineBoundsLocal = lineBounds,
      .textBoundsLocal = textBounds,
      .contentBoundsLocal = contentBounds,
      .frameBoundsLocal = frameBounds,
      .frameInnerBoundsLocal = frameInnerBounds,
      .frameContentBoundsLocal = frameContentBounds,
      .caretBoundsLocal = {
        .minimum = glm::dvec2(caretCenterX - halfCaretWidth, lineBounds.minimum.y),
        .maximum = glm::dvec2(caretCenterX + halfCaretWidth, lineBounds.maximum.y)
      },
      .selectionBoundsLocal = {},
      .caretSegmentsLocal = {
        .stemStart = glm::dvec2(caretCenterX, lineBounds.minimum.y),
        .stemEnd = glm::dvec2(caretCenterX, lineBounds.maximum.y),
        .topCapStart = glm::dvec2(caretCenterX - halfCapWidth, lineBounds.maximum.y),
        .topCapEnd = glm::dvec2(caretCenterX + halfCapWidth, lineBounds.maximum.y),
        .bottomCapStart = glm::dvec2(caretCenterX - halfCapWidth, lineBounds.minimum.y),
        .bottomCapEnd = glm::dvec2(caretCenterX + halfCapWidth, lineBounds.minimum.y)
      }
    };

    const int clampedSelectionStart = std::max(0, selectionStartByteOffset);
    const int clampedSelectionEnd = std::max(clampedSelectionStart, selectionEndByteOffset);
    if (clampedSelectionEnd > clampedSelectionStart) {
      const double selectionLeftX = boundaryX(textLayout, clampedSelectionStart);
      const double selectionRightX = boundaryX(textLayout, clampedSelectionEnd);
      layout.selectionBoundsLocal = {
        .minimum = glm::dvec2(std::min(selectionLeftX, selectionRightX), lineBounds.minimum.y),
        .maximum = glm::dvec2(std::max(selectionLeftX, selectionRightX), lineBounds.maximum.y)
      };
      layout.selectionBoundsLocal.minimum.x = std::max(layout.selectionBoundsLocal.minimum.x, frameContentBounds.minimum.x);
      layout.selectionBoundsLocal.maximum.x = std::min(layout.selectionBoundsLocal.maximum.x, frameContentBounds.maximum.x);
      layout.hasSelection = layout.selectionBoundsLocal.maximum.x > layout.selectionBoundsLocal.minimum.x;
    }

    return layout;
  }
} // Qadra::Core
