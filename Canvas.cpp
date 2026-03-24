#include "Canvas.hpp"

#include <QCursor>
#include <QFile>
#include <QMessageBox>
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QTextStream>
#include <QWheelEvent>
#include <glad/gl.h>
#include <stdexcept>

namespace
{
  int viewportPixels ( const int logicalPixels, const qreal devicePixelRatio )
  {
    return static_cast<int> (
        std::lround ( static_cast<double> ( logicalPixels ) * devicePixelRatio ) );
  }

  glm::dvec2 viewportPixels ( const QPointF &logicalPosition, const qreal devicePixelRatio )
  {
    return { logicalPosition.x () * devicePixelRatio, logicalPosition.y () * devicePixelRatio };
  }
} // namespace

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

  QString Canvas::loadShaderSource ( const QString &filename )
  {
    const QString path = QCoreApplication::applicationDirPath () + "/shaders/" + filename;
    QFile file ( path );

    if ( ! file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
    {
      throw std::runtime_error ( "Failed to open shader: " + path.toStdString () );
    }

    return QTextStream ( &file ).readAll ();
  }

  void Canvas::initializeGL ()
  {
    if ( gladLoadGL ( &Canvas::getProcAddress ) == 0 )
    {
      QMessageBox::critical ( this, tr ( "OpenGL" ), tr ( "Failed to load OpenGL functions." ) );
      return;
    }

    m_gridPass.emplace ();
    m_renderer.emplace ();
    m_font.emplace ( m_fontEngine, "C:/Windows/Fonts/comsc.ttf" );

    const QString gridVertexSource = loadShaderSource ( "grid.vertex.glsl" );
    const QString gridFragmentSource = loadShaderSource ( "grid.fragment.glsl" );

    m_gridPass->init ( gridVertexSource, gridFragmentSource );

    m_renderer->init ( QCoreApplication::applicationDirPath () + "/shaders" );

    m_initialized = true;

    m_document.addLine ( { glm::dvec2 ( -100.0, -100.0 ), glm::dvec2 ( 100.0, 100.0 ) } );
    for ( size_t i = 0; i < 500; i++ )
    {
      for ( size_t j = 0; j < 500; j++ )
      {
        m_document.addText (
            { glm::dvec2 ( i * 650, j * 60 ), "Hello Qadra Tarik EL JABIRI", 50.0 }, *m_font );
      }
    }
  }

  void Canvas::paintGL ()
  {
    if ( ! m_initialized )
    {
      return;
    }

    const qreal devicePixelRatioValue = devicePixelRatioF ();
    const GLsizei viewportWidthPixels = viewportPixels ( width (), devicePixelRatioValue );
    const GLsizei viewportHeightPixels = viewportPixels ( height (), devicePixelRatioValue );

    if ( viewportWidthPixels <= 0 || viewportHeightPixels <= 0 )
    {
      return;
    }

    if ( m_camera.width () != viewportWidthPixels || m_camera.height () != viewportHeightPixels )
    {
      if ( m_hasInitializedCameraViewport )
      {
        m_camera.resizePreserveViewportOrigin ( viewportWidthPixels, viewportHeightPixels );
      }
      else
      {
        m_camera.resize ( viewportWidthPixels, viewportHeightPixels );
        m_hasInitializedCameraViewport = true;
      }
    }

    glEnable ( GL_BLEND );
    glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable ( GL_MULTISAMPLE );

    glViewport ( 0, 0, viewportWidthPixels, viewportHeightPixels );
    glClearColor ( 0.09f, 0.10f, 0.12f, 1.0f );
    glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    m_gridPass->render ( m_camera, glm::vec2 ( static_cast<float> ( viewportWidthPixels ),
                                               static_cast<float> ( viewportHeightPixels ) ) );

    glEnable ( GL_DEPTH_TEST );
    m_renderer->render ( m_document, m_camera, *m_font );
    glDisable ( GL_DEPTH_TEST );
  }

  void Canvas::resizeGL ( const int, const int )
  {
    const qreal devicePixelRatioValue = devicePixelRatioF ();
    const int viewportWidthPixels = viewportPixels ( width (), devicePixelRatioValue );
    const int viewportHeightPixels = viewportPixels ( height (), devicePixelRatioValue );

    if ( m_hasInitializedCameraViewport )
    {
      m_camera.resizePreserveViewportOrigin ( viewportWidthPixels, viewportHeightPixels );
    }
    else
    {
      m_camera.resize ( viewportWidthPixels, viewportHeightPixels );
      m_hasInitializedCameraViewport = true;
    }
  }

  QFunctionPointer Canvas::getProcAddress ( const char *procName )
  {
    if ( const QOpenGLContext *context = QOpenGLContext::currentContext () )
    {
      return context->getProcAddress ( procName );
    }
    return nullptr;
  }

  void Canvas::mousePressEvent ( QMouseEvent *event )
  {
    if ( event->button () == Qt::MiddleButton )
    {
      setFocus ( Qt::MouseFocusReason );
      m_panning = true;
      const glm::dvec2 positionPixels = viewportPixels ( event->position (), devicePixelRatioF () );
      m_lastMousePosition = QPointF ( positionPixels.x, positionPixels.y );
      setCursor ( QCursor ( Qt::ClosedHandCursor ) );
      event->accept ();
      return;
    }

    QOpenGLWidget::mousePressEvent ( event );
  }

  void Canvas::mouseReleaseEvent ( QMouseEvent *event )
  {
    if ( event->button () == Qt::MiddleButton )
    {
      m_panning = false;
      unsetCursor ();
      event->accept ();
      return;
    }

    QOpenGLWidget::mouseReleaseEvent ( event );
  }

  void Canvas::mouseMoveEvent ( QMouseEvent *event )
  {
    const glm::dvec2 positionPixels = viewportPixels ( event->position (), devicePixelRatioF () );

    if ( m_panning )
    {
      const QPointF currentMousePosition ( positionPixels.x, positionPixels.y );
      const QPointF delta = currentMousePosition - m_lastMousePosition;
      m_camera.pan ( glm::dvec2 ( -delta.x (), delta.y () ) );
      m_lastMousePosition = currentMousePosition;
      update ();
      event->accept ();
      return;
    }

    QOpenGLWidget::mouseMoveEvent ( event );
  }

  void Canvas::wheelEvent ( QWheelEvent *event )
  {
    const float delta = event->angleDelta ().y () / 105.0f;
    const glm::dvec2 mouseScreenPixels =
        viewportPixels ( event->position (), devicePixelRatioF () );
    const glm::dvec2 mouseWorld = m_camera.screenToWorld ( mouseScreenPixels );
    m_camera.zoom ( std::pow ( 1.1f, delta ), mouseWorld );

    update ();
    event->accept ();
  }
} // namespace Qadra::Ui
