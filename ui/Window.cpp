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
  addToolBar ( Qt::TopToolBarArea, m_drawToolBar );

  m_canvas = new Qadra::Ui::Canvas ( this );
  setCentralWidget ( m_canvas );

  m_cmdLine = new Qadra::Ui::CmdLine ( this );
  addDockWidget ( Qt::BottomDockWidgetArea, m_cmdLine );

  auto *statusBar = new QStatusBar ( this );

  setStatusBar ( statusBar );
}

Window::~Window () = default;
