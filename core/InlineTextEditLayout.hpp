#ifndef QADRA_CORE_INLINETEXTEDITLAYOUT_HPP
#define QADRA_CORE_INLINETEXTEDITLAYOUT_HPP

#include <string>

#include <glm/glm.hpp>

#include "Font.hpp"
#include "TextLayout.hpp"

namespace Qadra::Core {
  struct InlineTextEditLayout {
    struct CaretSegments {
      glm::dvec2 stemStart{0.0};
      glm::dvec2 stemEnd{0.0};
      glm::dvec2 topCapStart{0.0};
      glm::dvec2 topCapEnd{0.0};
      glm::dvec2 bottomCapStart{0.0};
      glm::dvec2 bottomCapEnd{0.0};
    };

    TextLayoutBounds stableLineBoundsLocal{};
    TextLayoutBounds textBoundsLocal{};
    TextLayoutBounds contentBoundsLocal{};
    TextLayoutBounds frameBoundsLocal{};
    TextLayoutBounds frameInnerBoundsLocal{};
    TextLayoutBounds frameContentBoundsLocal{};
    TextLayoutBounds caretBoundsLocal{};
    TextLayoutBounds selectionBoundsLocal{};
    CaretSegments caretSegmentsLocal{};
    bool hasSelection{false};
  };

  [[nodiscard]] TextLayoutBounds defaultInlineEditingLineBounds(Font &font, double heightWorld);

  [[nodiscard]] InlineTextEditLayout buildInlineTextEditLayout(const TextLayout &textLayout,
                                                               double zoom,
                                                               const TextLayoutBounds &stableLineBoundsLocal,
                                                               int cursorByteOffset,
                                                               int selectionStartByteOffset,
                                                               int selectionEndByteOffset);
} // Qadra::Core

#endif // QADRA_CORE_INLINETEXTEDITLAYOUT_HPP
