#ifndef QADRA_UI_CANVAS_HPP
#define QADRA_UI_CANVAS_HPP

#include <glad/gl.h>

#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <QOpenGLWidget>
#include <QPointF>
#include <QRectF>
#include <QString>

#include "Camera.hpp"
#include "CanvasFontCatalog.hpp"
#include "DebugTextOverlay.hpp"
#include "Font.hpp"
#include "GridPass.hpp"
#include "InlineTextEditPresenter.hpp"
#include "InlineTextEditSessionController.hpp"
#include "LineRenderer.hpp"
#include "OrientedBoxRenderer.hpp"
#include "TextRenderer.hpp"

class QKeyEvent;
class QMouseEvent;
class QWheelEvent;

namespace Qadra::Core {
  class Document;
}

namespace Qadra::Ui {
  class CommandManager;

  class Canvas : public QOpenGLWidget {
    Q_OBJECT

  public:
    using InlineTextEditRequest = InlineTextEditSessionController::Request;
    using InlineTextEditCallbacks = InlineTextEditSessionController::Callbacks;

    explicit Canvas(QWidget *parent = nullptr);

    static QString loadShaderSource(const QString &filename);

    void setDocument(Qadra::Core::Document *document);

    void setCommandManager(CommandManager *commandManager);

    void setDebugTextOverlayEnabled(bool enabled);

    [[nodiscard]] std::string defaultFontKey() const;

    [[nodiscard]] glm::vec4 defaultTextColor() const noexcept;

    [[nodiscard]] Qadra::Core::Font *font(std::string_view fontKey);

    [[nodiscard]] const Qadra::Core::Font *font(std::string_view fontKey) const;

    void beginInlineTextEdit(const InlineTextEditRequest &request, InlineTextEditCallbacks callbacks);

    void updateInlineTextEdit(const InlineTextEditRequest &request);

    void endInlineTextEdit();

    [[nodiscard]] bool hasActiveInlineTextEdit() const noexcept;

  signals:
    void fontsChanged();

  protected:
    void initializeGL() override;

    void paintGL() override;

    void resizeGL(int width, int height) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

  private:
    void rebuildDebugTextOverlay();

    void renderTextEntities(std::span<const Qadra::Core::TextEntityData *const> textEntities,
                            const Qadra::Core::TextLayoutBounds &visibleWorldBounds);

    void renderInlineTextEdit();

    void updateInputMethodState();

    [[nodiscard]] std::optional<int> inlineCursorIndexAtWorldPosition(const glm::dvec2 &worldPosition);

    [[nodiscard]] QRectF inlineCaretRectangleLogical();

    static QFunctionPointer getProcAddress(const char *procName);

    bool m_initialized = false;
    bool m_hasInitializedCameraViewport = false;
    bool m_debugTextOverlayEnabled = false;
    Qadra::Core::Document *m_document = nullptr;
    CommandManager *m_commandManager = nullptr;

    Qadra::Core::Camera m_camera;
    QPointF m_lastMousePosition;
    bool m_panning{false};

    std::optional<Render::GridPass> m_gridPass;
    std::optional<Qadra::Core::LineRenderer> m_lineRenderer;
    std::optional<Qadra::Core::OrientedBoxRenderer> m_orientedBoxRenderer;
    std::optional<Qadra::Core::TextRenderer> m_textRenderer;
    CanvasFontCatalog m_fontCatalog;
    InlineTextEditPresenter m_inlineTextEditPresenter;
    DebugTextOverlay m_debugTextOverlay;
    std::unique_ptr<InlineTextEditSessionController> m_inlineTextEditController;
  };
} // Qadra::Ui

#endif // QADRA_UI_CANVAS_HPP
