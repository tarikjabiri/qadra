#include "Canvas.hpp"

#include <QCursor>
#include <QEnterEvent>
#include <QFile>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QPainter>
#include <QSurfaceFormat>
#include <QWheelEvent>
#include <glad/gl.h>

namespace
{
  [[nodiscard]] Qadra::Command::PointerButton toPointerButton ( const Qt::MouseButton button )
  {
    switch ( button )
    {
      case Qt::LeftButton:
        return Qadra::Command::PointerButton::Left;
      case Qt::MiddleButton:
        return Qadra::Command::PointerButton::Middle;
      case Qt::RightButton:
        return Qadra::Command::PointerButton::Right;
      default:
        return Qadra::Command::PointerButton::None;
    }
  }

  [[nodiscard]] Qadra::Command::PointerModifiers
  toPointerModifiers ( const Qt::KeyboardModifiers modifiers ) noexcept
  {
    Qadra::Command::PointerModifiers pointerModifiers;
    pointerModifiers.shift = modifiers.testFlag ( Qt::ShiftModifier );
    pointerModifiers.control = modifiers.testFlag ( Qt::ControlModifier );
    pointerModifiers.alt = modifiers.testFlag ( Qt::AltModifier );
    return pointerModifiers;
  }

  [[nodiscard]] bool shouldRedirectCommandInput ( const QKeyEvent &event ) noexcept
  {
    const Qt::KeyboardModifiers blockedModifiers =
        Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier;

    if ( ( event.modifiers () & blockedModifiers ) != 0 ) return false;
    if ( event.text ().isEmpty () ) return false;

    for ( const QChar ch : event.text () )
    {
      if ( ! ch.isPrint () ) return false;
    }

    return true;
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

    setFocusPolicy ( Qt::StrongFocus );
    setMouseTracking ( true );
  }

  void Canvas::startCommand ( const Tool::ToolKind kind )
  {
    applyCommandOutput ( m_commandManager.start ( kind, makeCommandContext () ) );
  }

  void Canvas::cancelCommand ()
  {
    applyCommandOutput ( m_commandManager.cancel ( makeCommandContext () ) );
  }

  void Canvas::setCommandInput ( const QString &text )
  {
    m_commandManager.setInput ( text.toStdString () );
    emit commandViewChanged ();
  }

  void Canvas::submitCommandInput ()
  {
    applyCommandOutput ( m_commandManager.submit ( makeCommandContext () ) );
  }

  Tool::ToolKind Canvas::activeToolKind () const noexcept
  {
    return m_commandManager.activeToolKind ();
  }

  Command::View Canvas::commandView () const { return m_commandManager.view (); }

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
    for ( size_t i = 0; i < 3; i++ )
    {
      for ( size_t j = 0; j < 30; j++ )
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

    const auto previewLines = makePreviewLines ();
    m_renderer->render ( m_document, m_camera, *m_font, previewLines );

    QPainter painter ( this );
    m_cursorOverlay.paint ( painter, makeCursorOverlayState () );
  }

  void Canvas::resizeGL ( const int, const int ) { updateCameraViewport (); }

  void Canvas::enterEvent ( QEnterEvent *event )
  {
    m_isMouseInside = true;
    updateCursorPosition ( event->position () );
    syncCanvasCursor ();
    update ();
    QOpenGLWidget::enterEvent ( event );
  }

  void Canvas::leaveEvent ( QEvent *event )
  {
    m_isMouseInside = false;
    m_hasCursorPosition = false;
    syncCanvasCursor ();
    update ();
    QOpenGLWidget::leaveEvent ( event );
  }

  QFunctionPointer Canvas::getProcAddress ( const char *procName )
  {
    if ( const QOpenGLContext *context = QOpenGLContext::currentContext () )
    {
      return context->getProcAddress ( procName );
    }
    return nullptr;
  }

  Command::Context Canvas::makeCommandContext ()
  {
    return Command::Context{ m_document, m_camera, m_font ? &*m_font : nullptr };
  }

  void Canvas::applyCommandOutput ( const Command::Output &output )
  {
    if ( output.requestRepaint ) update ();
    if ( output.viewChanged ) emit commandViewChanged ();
  }

  Command::PointerEvent Canvas::makeCommandPointerEvent ( const QMouseEvent &event )
  {
    updateCameraViewport ();

    const glm::dvec2 screenPosition = m_camera.devicePixels ( event.position () );
    Command::PointerEvent pointerEvent;
    pointerEvent.screenPosition = screenPosition;
    pointerEvent.worldPosition = m_camera.screenToWorld ( screenPosition );
    pointerEvent.button = toPointerButton ( event.button () );
    pointerEvent.modifiers = toPointerModifiers ( event.modifiers () );
    return pointerEvent;
  }

  std::vector<Render::PreviewLine> Canvas::makePreviewLines () const
  {
    const Command::Preview preview = m_commandManager.preview ();
    std::vector<Render::PreviewLine> lines;
    lines.reserve ( preview.lines.size () );

    for ( const auto &line : preview.lines )
    {
      lines.push_back ( Render::PreviewLine{
          .start = line.start,
          .end = line.end,
          .color = line.color,
      } );
    }

    return lines;
  }

  CanvasCursorOverlay::State Canvas::makeCursorOverlayState () const
  {
    return CanvasCursorOverlay::State{
        .position = m_cursorPosition,
        .visible = shouldUseCustomCursor () && m_hasCursorPosition,
        .showPickbox = shouldShowCursorPickbox (),
    };
  }

  bool Canvas::shouldUseCustomCursor () const noexcept
  {
    return m_isMouseInside && ! m_cameraController.isPanning ();
  }

  bool Canvas::shouldShowCursorPickbox () const noexcept
  {
    return m_commandManager.activeToolKind () == Tool::ToolKind::None;
  }

  void Canvas::syncCanvasCursor ()
  {
    if ( m_cameraController.isPanning () )
    {
      setCursor ( Qt::ClosedHandCursor );
      return;
    }

    if ( shouldUseCustomCursor () )
    {
      setCursor ( Qt::BlankCursor );
      return;
    }

    unsetCursor ();
  }

  void Canvas::updateCursorPosition ( const QPointF &position )
  {
    m_cursorPosition = position;
    m_hasCursorPosition = true;
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
    setFocus ( Qt::MouseFocusReason );
    updateCursorPosition ( event->position () );

    if ( event->button () == Qt::MiddleButton )
    {
      const auto position = m_camera.devicePixels ( event->position () );
      m_cameraController.mousePress ( position, event->button () );
      syncCanvasCursor ();
      update ();
      event->accept ();
      return;
    }

    const Command::Output output =
        m_commandManager.pointerPress ( makeCommandContext (), makeCommandPointerEvent ( *event ) );

    applyCommandOutput ( output );

    if ( output.handled )
    {
      event->accept ();
      return;
    }

    QOpenGLWidget::mousePressEvent ( event );
  }

  void Canvas::mouseReleaseEvent ( QMouseEvent *event )
  {
    updateCursorPosition ( event->position () );

    if ( event->button () == Qt::MiddleButton )
    {
      m_cameraController.mouseRelease ( event->button () );
      syncCanvasCursor ();
      update ();
      event->accept ();
      return;
    }

    const Command::Output output = m_commandManager.pointerRelease (
        makeCommandContext (), makeCommandPointerEvent ( *event ) );

    applyCommandOutput ( output );

    if ( output.handled )
    {
      event->accept ();
      return;
    }

    QOpenGLWidget::mouseReleaseEvent ( event );
  }

  void Canvas::mouseMoveEvent ( QMouseEvent *event )
  {
    updateCursorPosition ( event->position () );

    const auto position = m_camera.devicePixels ( event->position () );
    m_cameraController.mouseMove ( position );

    if ( m_cameraController.isPanning () )
    {
      syncCanvasCursor ();
      update ();
      event->accept ();
      return;
    }

    const Command::Output output =
        m_commandManager.pointerMove ( makeCommandContext (), makeCommandPointerEvent ( *event ) );

    syncCanvasCursor ();
    applyCommandOutput ( output );
    update ();

    if ( output.handled )
    {
      event->accept ();
      return;
    }

    QOpenGLWidget::mouseMoveEvent ( event );
  }

  void Canvas::keyPressEvent ( QKeyEvent *event )
  {
    if ( event->key () == Qt::Key_Escape )
    {
      const Command::Output output = m_commandManager.cancel ( makeCommandContext () );
      applyCommandOutput ( output );

      if ( output.handled )
      {
        event->accept ();
        return;
      }
    }

    if ( event->key () == Qt::Key_Backspace &&
         ( event->modifiers () & ( Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier ) ) ==
             0 )
    {
      m_commandManager.backspaceInput ();
      emit commandViewChanged ();
      event->accept ();
      return;
    }

    if ( event->key () == Qt::Key_Return || event->key () == Qt::Key_Enter )
    {
      applyCommandOutput ( m_commandManager.submit ( makeCommandContext () ) );
      event->accept ();
      return;
    }

    if ( shouldRedirectCommandInput ( *event ) )
    {
      m_commandManager.appendInput ( event->text ().toStdString () );
      emit commandViewChanged ();
      event->accept ();
      return;
    }

    QOpenGLWidget::keyPressEvent ( event );
  }

  void Canvas::wheelEvent ( QWheelEvent *event )
  {
    updateCursorPosition ( event->position () );
    const float delta = event->angleDelta ().y () / 120.0f;
    const auto pos = m_camera.devicePixels ( event->position () );
    m_cameraController.wheel ( delta, pos );
    update ();
    event->accept ();
  }
} // namespace Qadra::Ui
