#include "Canvas.hpp"

#include <QCursor>
#include <QFile>
#include <QMessageBox>
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QWheelEvent>
#include <glad/gl.h>

namespace Qadra::Ui
{
  Canvas::Canvas ( QWidget *parent ) : QOpenGLWidget ( parent )
  {
    QSurfaceFormat format;
    format.setProfile ( QSurfaceFormat::CoreProfile );
    format.setVersion ( 4, 6 );
    format.setAlphaBufferSize ( 8 );
    format.setSwapInterval ( 0 );
    format.setDepthBufferSize ( 24 );
    setFormat ( format );

    // setFocusPolicy ( Qt::StrongFocus );
    // setMouseTracking ( true );
  }

  void Canvas::initializeGL ()
  {
    if ( gladLoadGL ( &Canvas::getProcAddress ) == 0 )
    {
      QMessageBox::critical ( this, tr ( "OpenGL" ), tr ( "Failed to load OpenGL functions." ) );
      return;
    }

    m_renderer.emplace ();
    m_font.emplace (
        m_fontEngine,
        "C:/Program Files/JetBrains/CLion 2025.3.3/jbr/lib/fonts/FiraCode-Retina.ttf" );

    m_renderer->init ();

    m_initialized = true;

    m_document.addLine ( { { -100.0, -100.0 }, { 100.0, 100.0 } } );
    for ( size_t i = 0; i < 40; i++ )
    {
      for ( size_t j = 0; j < 200; j++ )
      {
        m_document.addText (
            { { i * 1650, j * 120 }, "Hello Qadra, Developed using OpenGL/Qt", 50.0 }, *m_font );
      }
    }
  }

  void Canvas::paintGL ()
  {
    if ( ! m_initialized ) return;

    updateCameraViewport ();

    m_renderer->render ( m_document, m_camera, *m_font );
  }

  void Canvas::resizeGL ( const int, const int ) { updateCameraViewport (); }

  QFunctionPointer Canvas::getProcAddress ( const char *procName )
  {
    if ( const QOpenGLContext *context = QOpenGLContext::currentContext () )
    {
      return context->getProcAddress ( procName );
    }
    return nullptr;
  }

  void Canvas::updateCameraViewport ()
  {
    m_camera.setDevicePixelRatio ( devicePixelRatioF () );

    const int w = m_camera.devicePixels ( width () );
    const int h = m_camera.devicePixels ( height () );

    if ( w <= 0 || h <= 0 ) return;
    if ( w == m_camera.width () && h == m_camera.height () ) return;

    if ( m_hasInitializedCameraViewport ) m_camera.resizePreserveViewportOrigin ( w, h );
    else
    {
      m_camera.resize ( w, h );
      m_hasInitializedCameraViewport = true;
    }
  }

  void Canvas::mousePressEvent ( QMouseEvent *event )
  {
    if ( event->button () == Qt::MiddleButton )
    {
      const auto position = m_camera.devicePixels ( event->position () );
      m_cameraController.mousePress ( position, event->button () );
      setCursor ( Qt::ClosedHandCursor );
      event->accept ();
      return;
    }

    QOpenGLWidget::mousePressEvent ( event );
  }

  void Canvas::mouseReleaseEvent ( QMouseEvent *event )
  {
    if ( event->button () == Qt::MiddleButton )
    {
      m_cameraController.mouseRelease ( event->button () );
      unsetCursor ();
      event->accept ();
      return;
    }

    QOpenGLWidget::mouseReleaseEvent ( event );
  }

  void Canvas::mouseMoveEvent ( QMouseEvent *event )
  {
    const auto position = m_camera.devicePixels ( event->position () );
    m_cameraController.mouseMove ( position );

    if ( m_cameraController.isPanning () )
    {
      update ();
      event->accept ();
      return;
    }

    QOpenGLWidget::mouseMoveEvent ( event );
  }

  void Canvas::wheelEvent ( QWheelEvent *event )
  {
    const float delta = event->angleDelta ().y () / 120.0f;
    const auto pos = m_camera.devicePixels ( event->position () );
    m_cameraController.wheel ( delta, pos );
    update ();
    event->accept ();
  }
} // namespace Qadra::Ui
