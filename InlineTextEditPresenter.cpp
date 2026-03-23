#include "InlineTextEditPresenter.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

#include "LineRenderer.hpp"
#include "OrientedBoxRenderer.hpp"
#include "TextRenderer.hpp"

namespace {
  constexpr double inlineCaretHaloThicknessPixels = 3.0;
  const glm::vec4 inlineEditingFrameGlowColor(0.86f, 0.91f, 0.98f, 0.07f);
  const glm::vec4 inlineEditingFrameBorderColor(0.88f, 0.92f, 0.98f, 0.24f);
  const glm::vec4 inlineEditingFrameFillColor(0.08f, 0.10f, 0.13f, 0.88f);
  const glm::vec4 inlineEditingSelectionColor(0.22f, 0.37f, 0.60f, 0.52f);
  const glm::vec4 inlineCaretHaloColor(0.03f, 0.04f, 0.06f, 0.96f);

  [[nodiscard]] bool hasVisibleInkBounds(const Qadra::Core::TextLayoutGlyph &glyph) {
    return glyph.inkBounds.maximum.x > glyph.inkBounds.minimum.x &&
           glyph.inkBounds.maximum.y > glyph.inkBounds.minimum.y;
  }

  [[nodiscard]] double boundaryX(const Qadra::Core::TextLayout &textLayout, const int byteOffset) {
    std::optional<double> leftBoundary;
    std::optional<double> rightBoundary;

    for (const Qadra::Core::TextLayoutGlyph &glyph: textLayout.glyphs) {
      const double glyphMinimumX = hasVisibleInkBounds(glyph) ? glyph.inkBounds.minimum.x : glyph.origin.x;
      const double glyphMaximumX = hasVisibleInkBounds(glyph) ? glyph.inkBounds.maximum.x : glyph.origin.x + glyph.advance.x;

      if (static_cast<int>(glyph.cluster) < byteOffset) {
        leftBoundary = glyphMaximumX;
        continue;
      }

      rightBoundary = glyphMinimumX;
      break;
    }

    if (leftBoundary) {
      return *leftBoundary;
    }
    if (rightBoundary) {
      return *rightBoundary;
    }

    return 0.0;
  }
}

namespace Qadra::Ui {
  int InlineTextEditPresenter::utf8ByteOffsetForStringIndex(const QString &text, const int stringIndex) {
    const int textLength = static_cast<int>(text.size());
    const int clampedIndex = std::clamp(stringIndex, 0, textLength);
    return static_cast<int>(text.left(clampedIndex).toUtf8().size());
  }

  glm::dvec2 InlineTextEditPresenter::inlineLocalToWorld(const InlineTextEditSessionController::Snapshot &snapshot,
                                                         const glm::dvec2 &localPosition) {
    if (std::abs(snapshot.rotationRadians) <= 1e-12) {
      return snapshot.worldPosition + localPosition;
    }

    const double sine = std::sin(snapshot.rotationRadians);
    const double cosine = std::cos(snapshot.rotationRadians);
    return snapshot.worldPosition + glm::dvec2(
      localPosition.x * cosine - localPosition.y * sine,
      localPosition.x * sine + localPosition.y * cosine
    );
  }

  glm::dvec2 InlineTextEditPresenter::worldToInlineLocal(const InlineTextEditSessionController::Snapshot &snapshot,
                                                         const glm::dvec2 &worldPosition) {
    const glm::dvec2 delta = worldPosition - snapshot.worldPosition;
    if (std::abs(snapshot.rotationRadians) <= 1e-12) {
      return delta;
    }

    const double sine = std::sin(snapshot.rotationRadians);
    const double cosine = std::cos(snapshot.rotationRadians);
    return {
      delta.x * cosine + delta.y * sine,
      -delta.x * sine + delta.y * cosine
    };
  }

  InlineTextEditPresenter::CaretLineSegments InlineTextEditPresenter::caretLineSegments(
    const Qadra::Core::InlineTextEditLayout &layout,
    const InlineTextEditSessionController::Snapshot &snapshot) {
    return {
      .stemStart = inlineLocalToWorld(snapshot, layout.caretSegmentsLocal.stemStart),
      .stemEnd = inlineLocalToWorld(snapshot, layout.caretSegmentsLocal.stemEnd),
      .topCapStart = inlineLocalToWorld(snapshot, layout.caretSegmentsLocal.topCapStart),
      .topCapEnd = inlineLocalToWorld(snapshot, layout.caretSegmentsLocal.topCapEnd),
      .bottomCapStart = inlineLocalToWorld(snapshot, layout.caretSegmentsLocal.bottomCapStart),
      .bottomCapEnd = inlineLocalToWorld(snapshot, layout.caretSegmentsLocal.bottomCapEnd)
    };
  }

  std::array<glm::dvec2, 4> InlineTextEditPresenter::lineSegmentRectangleCorners(const glm::dvec2 &from,
                                                                                const glm::dvec2 &to,
                                                                                const double thicknessWorld) {
    const glm::dvec2 delta = to - from;
    const double length = std::sqrt(delta.x * delta.x + delta.y * delta.y);
    if (length <= 1e-12) {
      const double halfThickness = 0.5 * thicknessWorld;
      return {
        from + glm::dvec2(-halfThickness, -halfThickness),
        from + glm::dvec2(halfThickness, -halfThickness),
        from + glm::dvec2(halfThickness, halfThickness),
        from + glm::dvec2(-halfThickness, halfThickness)
      };
    }

    const glm::dvec2 tangent = delta / length;
    const glm::dvec2 normal(-tangent.y, tangent.x);
    const glm::dvec2 halfNormal = normal * (0.5 * thicknessWorld);
    const glm::dvec2 halfTangent = tangent * (0.5 * thicknessWorld);

    return {
      from - halfTangent - halfNormal,
      from - halfTangent + halfNormal,
      to + halfTangent + halfNormal,
      to + halfTangent - halfNormal
    };
  }

  InlineTextEditPresenter::VisualState InlineTextEditPresenter::buildVisualState(
    Qadra::Core::Font &font,
    const InlineTextEditSessionController::Snapshot &snapshot,
    const double zoom) {
    const std::string displayTextUtf8 = snapshot.displayText.toUtf8().toStdString();
    const Qadra::Core::TextLayout textLayout = Qadra::Core::measureTextLayout(font, displayTextUtf8, snapshot.heightWorld);
    return {
      .textLayout = textLayout,
      .editLayout = Qadra::Core::buildInlineTextEditLayout(
        textLayout,
        zoom,
        snapshot.stableLineBoundsLocal,
        utf8ByteOffsetForStringIndex(snapshot.displayText, snapshot.cursorIndex),
        utf8ByteOffsetForStringIndex(snapshot.displayText, snapshot.selectionStart),
        utf8ByteOffsetForStringIndex(snapshot.displayText, snapshot.selectionEnd)
      )
    };
  }

  glm::vec4 InlineTextEditPresenter::caretColor(const glm::vec4 &color) {
    return {
      color.r,
      color.g,
      color.b,
      std::max(0.8f, color.a)
    };
  }

  void InlineTextEditPresenter::render(const InlineTextEditSessionController::Snapshot &snapshot,
                                       Qadra::Core::Font &font,
                                       const RenderResources &resources) const {
    const double zoom = std::max(resources.camera.zoom(), 1e-6);
    const VisualState visualState = buildVisualState(font, snapshot, zoom);
    const Qadra::Core::InlineTextEditLayout &layoutData = visualState.editLayout;

    resources.orientedBoxRenderer.begin(resources.camera);

    resources.orientedBoxRenderer.draw(
      Qadra::Core::orientedTextCorners(snapshot.worldPosition, layoutData.frameBoundsLocal, snapshot.rotationRadians),
      inlineEditingFrameGlowColor
    );
    resources.orientedBoxRenderer.draw(
      Qadra::Core::orientedTextCorners(snapshot.worldPosition, layoutData.frameInnerBoundsLocal, snapshot.rotationRadians),
      inlineEditingFrameBorderColor
    );
    resources.orientedBoxRenderer.draw(
      Qadra::Core::orientedTextCorners(snapshot.worldPosition, layoutData.frameContentBoundsLocal, snapshot.rotationRadians),
      inlineEditingFrameFillColor
    );

    if (layoutData.hasSelection) {
      resources.orientedBoxRenderer.draw(
        Qadra::Core::orientedTextCorners(snapshot.worldPosition, layoutData.selectionBoundsLocal, snapshot.rotationRadians),
        inlineEditingSelectionColor
      );
    }

    resources.orientedBoxRenderer.end();

    if (!snapshot.displayText.isEmpty()) {
      resources.textRenderer.begin(font, resources.camera);
      resources.textRenderer.draw(
        snapshot.displayText.toUtf8().toStdString(),
        snapshot.worldPosition,
        snapshot.heightWorld,
        snapshot.rotationRadians,
        snapshot.color
      );
      resources.textRenderer.end();
    }

    if (!snapshot.caretVisible) {
      return;
    }

    const CaretLineSegments caretSegments = caretLineSegments(layoutData, snapshot);
    const double caretHaloThicknessWorld = inlineCaretHaloThicknessPixels / zoom;

    resources.orientedBoxRenderer.begin(resources.camera);
    resources.orientedBoxRenderer.draw(
      lineSegmentRectangleCorners(caretSegments.stemStart, caretSegments.stemEnd, caretHaloThicknessWorld),
      inlineCaretHaloColor
    );
    resources.orientedBoxRenderer.draw(
      lineSegmentRectangleCorners(caretSegments.topCapStart, caretSegments.topCapEnd, caretHaloThicknessWorld),
      inlineCaretHaloColor
    );
    resources.orientedBoxRenderer.draw(
      lineSegmentRectangleCorners(caretSegments.bottomCapStart, caretSegments.bottomCapEnd, caretHaloThicknessWorld),
      inlineCaretHaloColor
    );
    resources.orientedBoxRenderer.end();

    resources.lineRenderer.begin(resources.camera, resources.viewportSizePixels);
    resources.lineRenderer.draw(caretSegments.stemStart, caretSegments.stemEnd, caretColor(snapshot.color));
    resources.lineRenderer.draw(caretSegments.topCapStart, caretSegments.topCapEnd, caretColor(snapshot.color));
    resources.lineRenderer.draw(caretSegments.bottomCapStart, caretSegments.bottomCapEnd, caretColor(snapshot.color));
    resources.lineRenderer.end();
  }

  std::optional<int> InlineTextEditPresenter::cursorIndexAtWorldPosition(
    const InlineTextEditSessionController::Snapshot &snapshot,
    const glm::dvec2 &worldPosition,
    Qadra::Core::Font &font) const {
    if (snapshot.displayText.isEmpty()) {
      return 0;
    }

    const VisualState visualState = buildVisualState(font, snapshot, 1.0);
    const glm::dvec2 localPosition = worldToInlineLocal(snapshot, worldPosition);
    const double clickX = localPosition.x;

    int bestIndex = 0;
    double bestDistance = std::abs(clickX - boundaryX(visualState.textLayout, 0));
    const int displayTextLength = static_cast<int>(snapshot.displayText.size());
    for (int index = 1; index <= displayTextLength; ++index) {
      const double caretBoundaryX = boundaryX(visualState.textLayout, utf8ByteOffsetForStringIndex(snapshot.displayText, index));
      const double distance = std::abs(clickX - caretBoundaryX);
      if (distance < bestDistance) {
        bestDistance = distance;
        bestIndex = index;
      }
    }

    return bestIndex;
  }

  QRectF InlineTextEditPresenter::caretRectangleLogical(const InlineTextEditSessionController::Snapshot &snapshot,
                                                        Qadra::Core::Font &font,
                                                        const Qadra::Core::Camera &camera,
                                                        const qreal devicePixelRatio) const {
    const double zoom = std::max(camera.zoom(), 1e-6);
    const VisualState visualState = buildVisualState(font, snapshot, zoom);

    const auto corners = Qadra::Core::orientedTextCorners(
      snapshot.worldPosition,
      visualState.editLayout.caretBoundsLocal,
      snapshot.rotationRadians
    );
    Qadra::Core::TextLayoutBounds screenBounds{
      .minimum = glm::dvec2(std::numeric_limits<double>::infinity()),
      .maximum = glm::dvec2(-std::numeric_limits<double>::infinity())
    };
    for (const glm::dvec2 &corner: corners) {
      const glm::dvec2 screen = camera.worldToScreen(corner);
      screenBounds.minimum.x = std::min(screenBounds.minimum.x, screen.x);
      screenBounds.minimum.y = std::min(screenBounds.minimum.y, screen.y);
      screenBounds.maximum.x = std::max(screenBounds.maximum.x, screen.x);
      screenBounds.maximum.y = std::max(screenBounds.maximum.y, screen.y);
    }

    return {
      screenBounds.minimum.x / devicePixelRatio,
      screenBounds.minimum.y / devicePixelRatio,
      (screenBounds.maximum.x - screenBounds.minimum.x) / devicePixelRatio,
      (screenBounds.maximum.y - screenBounds.minimum.y) / devicePixelRatio
    };
  }
} // Qadra::Ui
