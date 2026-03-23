#ifndef QADRA_UI_INLINETEXTEDITPRESENTER_HPP
#define QADRA_UI_INLINETEXTEDITPRESENTER_HPP

#include <optional>
#include <array>

#include <QRectF>
#include <QString>

#include <glm/glm.hpp>

#include "Camera.hpp"
#include "Font.hpp"
#include "InlineTextEditLayout.hpp"
#include "InlineTextEditSessionController.hpp"

namespace Qadra::Core {
  class LineRenderer;
  class OrientedBoxRenderer;
  class TextRenderer;
}

namespace Qadra::Ui {
  class InlineTextEditPresenter {
  public:
    struct RenderResources {
      const Qadra::Core::Camera &camera;
      Qadra::Core::TextRenderer &textRenderer;
      Qadra::Core::OrientedBoxRenderer &orientedBoxRenderer;
      Qadra::Core::LineRenderer &lineRenderer;
      glm::vec2 viewportSizePixels{0.0f};
    };

    void render(const InlineTextEditSessionController::Snapshot &snapshot,
                Qadra::Core::Font &font,
                const RenderResources &resources) const;

    [[nodiscard]] std::optional<int> cursorIndexAtWorldPosition(const InlineTextEditSessionController::Snapshot &snapshot,
                                                                const glm::dvec2 &worldPosition,
                                                                Qadra::Core::Font &font) const;

    [[nodiscard]] QRectF caretRectangleLogical(const InlineTextEditSessionController::Snapshot &snapshot,
                                               Qadra::Core::Font &font,
                                               const Qadra::Core::Camera &camera,
                                               qreal devicePixelRatio) const;

  private:
    struct VisualState {
      Qadra::Core::TextLayout textLayout;
      Qadra::Core::InlineTextEditLayout editLayout;
    };

    struct CaretLineSegments {
      glm::dvec2 stemStart{0.0};
      glm::dvec2 stemEnd{0.0};
      glm::dvec2 topCapStart{0.0};
      glm::dvec2 topCapEnd{0.0};
      glm::dvec2 bottomCapStart{0.0};
      glm::dvec2 bottomCapEnd{0.0};
    };

    [[nodiscard]] static int utf8ByteOffsetForStringIndex(const QString &text, int stringIndex);

    [[nodiscard]] static glm::dvec2 inlineLocalToWorld(const InlineTextEditSessionController::Snapshot &snapshot,
                                                       const glm::dvec2 &localPosition);

    [[nodiscard]] static glm::dvec2 worldToInlineLocal(const InlineTextEditSessionController::Snapshot &snapshot,
                                                       const glm::dvec2 &worldPosition);

    [[nodiscard]] static CaretLineSegments caretLineSegments(const Qadra::Core::InlineTextEditLayout &layout,
                                                             const InlineTextEditSessionController::Snapshot &snapshot);

    [[nodiscard]] static std::array<glm::dvec2, 4> lineSegmentRectangleCorners(const glm::dvec2 &from,
                                                                               const glm::dvec2 &to,
                                                                               double thicknessWorld);

    [[nodiscard]] static VisualState buildVisualState(Qadra::Core::Font &font,
                                                      const InlineTextEditSessionController::Snapshot &snapshot,
                                                      double zoom);

    [[nodiscard]] static glm::vec4 caretColor(const glm::vec4 &color);
  };
} // Qadra::Ui

#endif // QADRA_UI_INLINETEXTEDITPRESENTER_HPP
