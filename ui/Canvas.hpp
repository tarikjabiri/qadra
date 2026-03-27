#ifndef QADRA_UI_CANVAS_HPP
#define QADRA_UI_CANVAS_HPP

#include "CameraController.hpp"
#include "CanvasCursorOverlay.hpp"
#include "Document.hpp"
#include "Renderer.hpp"
#include "ToolContext.hpp"
#include "ToolKind.hpp"
#include "ToolManager.hpp"

#include <QOpenGLWidget>
#include <QString>
#include <optional>
#include <vector>

class QMouseEvent;
class QEnterEvent;
class QEvent;
class QWheelEvent;

namespace Qadra::Ui
{
  class Canvas : public QOpenGLWidget
  {
    Q_OBJECT

  public:
    explicit Canvas ( QWidget *parent = nullptr );

    void setActiveTool ( Qadra::Tool::ToolKind kind );

  protected:
    void initializeGL () override;

    void paintGL () override;

    void resizeGL ( int width, int height ) override;

    void enterEvent ( QEnterEvent *event ) override;

    void leaveEvent ( QEvent *event ) override;

    void mousePressEvent ( QMouseEvent *event ) override;

    void mouseReleaseEvent ( QMouseEvent *event ) override;

    void mouseMoveEvent ( QMouseEvent *event ) override;

    void wheelEvent ( QWheelEvent *event ) override;

  private:
    static QFunctionPointer getProcAddress ( const char *procName );

    [[nodiscard]] Qadra::Tool::ToolContext makeToolContext ();

    void applyToolEventResult ( const Qadra::Tool::ToolEventResult &result );

    [[nodiscard]] Qadra::Tool::ToolPointerEvent makeToolPointerEvent ( const QMouseEvent &event );

    [[nodiscard]] std::vector<Qadra::Render::PreviewLine> makePreviewLines ();
    [[nodiscard]] CanvasCursorOverlay::State makeCursorOverlayState () const;
    [[nodiscard]] bool shouldUseCustomCursor () const noexcept;
    [[nodiscard]] bool shouldShowCursorPickbox () const noexcept;

    void syncCanvasCursor ();
    void updateCursorPosition ( const QPointF &position );

    void updateCameraViewport ();

    bool m_initialized = false;
    bool m_hasInitializedCameraViewport = false;

    Core::Camera m_camera;
    Core::CameraController m_cameraController{ m_camera };

    Cad::Document m_document;
    Qadra::Tool::ToolManager m_toolManager;
    std::optional<Render::Renderer> m_renderer;
    CanvasCursorOverlay m_cursorOverlay;

    Core::FontEngine m_fontEngine;
    std::optional<Core::Font> m_font;

    QPointF m_cursorPosition;
    bool m_hasCursorPosition = false;
    bool m_isMouseInside = false;
  };
} // namespace Qadra::Ui

#endif // QADRA_UI_CANVAS_HPP
