#include "Window.hpp"

#include "Canvas.hpp"

#include <QAction>
#include <QIcon>
#include <QMenuBar>

Window::Window ( QWidget *parent ) : QMainWindow ( parent )
{
  setWindowTitle ( "Qadra" );
  setWindowIcon ( QIcon ( ":/icons/qadra-icon.svg" ) );
  resize ( 1200, 800 );

  m_canvas = new Qadra::Ui::Canvas ( this );
  setCentralWidget ( m_canvas );
}

Window::~Window () = default;
