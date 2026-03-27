#include "Window.hpp"

#include "Canvas.hpp"
#include "CmdLine.hpp"
#include "DrawToolBar.hpp"

#include <QIcon>
#include <QStatusBar>

Window::Window ( QWidget *parent ) : QMainWindow ( parent )
{
  setWindowTitle ( "Qadra" );
  setWindowIcon ( QIcon ( ":/icons/qadra-icon.svg" ) );
  resize ( 1200, 800 );

  m_drawToolBar = new Qadra::Ui::DrawToolBar ( this );
  addToolBar ( Qt::LeftToolBarArea, m_drawToolBar );

  m_canvas = new Qadra::Ui::Canvas ( this );
  setCentralWidget ( m_canvas );

  m_cmdLine = new Qadra::Ui::CmdLine ( this );
  addDockWidget ( Qt::BottomDockWidgetArea, m_cmdLine );

  connect ( m_drawToolBar, &Qadra::Ui::DrawToolBar::toolSelected, this, &Window::selectTool );
  connect ( m_canvas, &Qadra::Ui::Canvas::commandViewChanged, this, &Window::syncCommandUi );
  connect ( m_cmdLine, &Qadra::Ui::CmdLine::inputEdited, m_canvas,
            &Qadra::Ui::Canvas::setCommandInput );
  connect ( m_cmdLine, &Qadra::Ui::CmdLine::submitRequested, m_canvas,
            &Qadra::Ui::Canvas::submitCommandInput );

  auto *statusBar = new QStatusBar ( this );
  setStatusBar ( statusBar );

  syncCommandUi ();
}

Window::~Window () = default;

void Window::selectTool ( const Qadra::Tool::ToolKind kind )
{
  if ( kind == Qadra::Tool::ToolKind::None ) m_canvas->cancelCommand ();
  else
    m_canvas->startCommand ( kind );

  m_canvas->setFocus ( Qt::OtherFocusReason );
  syncCommandUi ();
}

void Window::syncCommandUi ()
{
  m_drawToolBar->setSelectedToolKind ( m_canvas->activeToolKind () );
  m_cmdLine->render ( m_canvas->commandView () );
}
