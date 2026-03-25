#ifndef QADRA_UI_CANVAS_HPP
#define QADRA_UI_CANVAS_HPP

#include "CameraController.hpp"
#include "Document.hpp"
#include "Renderer.hpp"

#include <QOpenGLWidget>
#include <QString>
#include <optional>

class QMouseEvent;
class QWheelEvent;

namespace Qadra::Ui
{
  class Canvas : public QOpenGLWidget
  {
    Q_OBJECT

  public:
    explicit Canvas ( QWidget *parent = nullptr );

  protected:
    void initializeGL () override;

    void paintGL () override;

    void resizeGL ( int width, int height ) override;

    void mousePressEvent ( QMouseEvent *event ) override;

    void mouseReleaseEvent ( QMouseEvent *event ) override;

    void mouseMoveEvent ( QMouseEvent *event ) override;

    void wheelEvent ( QWheelEvent *event ) override;

  private:
    static QFunctionPointer getProcAddress ( const char *procName );

    void updateCameraViewport ();

    bool m_initialized = false;
    bool m_hasInitializedCameraViewport = false;

    Core::Camera m_camera;
    Core::CameraController m_cameraController{ m_camera };

    Cad::Document m_document;
    std::optional<Render::Renderer> m_renderer;

    Core::FontEngine m_fontEngine;
    std::optional<Core::Font> m_font;
  };
} // namespace Qadra::Ui

#endif // QADRA_UI_CANVAS_HPP
