#include "Window.hpp"

#include "Canvas.hpp"
#include "CmdLine.hpp"
#include "DrawMenu.hpp"
#include "DrawToolBar.hpp"
#include "QuickToolBar.hpp"

#include <QAction>
#include <QIcon>
#include <QKeySequence>
#include <QMenuBar>
#include <QStatusBar>
#include <QUndoStack>

Window::Window ( QWidget *parent ) : QMainWindow ( parent )
{
  setWindowTitle ( "Qadra" );
  setWindowIcon ( QIcon ( ":/icons/qadra-icon.svg" ) );
  resize ( 1200, 800 );

  m_editMenu = menuBar ()->addMenu ( tr ( "&Edit" ) );
  m_undoAction = m_editMenu->addAction ( tr ( "&Undo" ) );
  m_redoAction = m_editMenu->addAction ( tr ( "&Redo" ) );
  m_undoAction->setIcon ( QIcon ( ":/assets/undo.svg" ) );
  m_redoAction->setIcon ( QIcon ( ":/assets/redo.svg" ) );
  m_undoAction->setShortcuts ( QKeySequence::Undo );
  m_redoAction->setShortcuts ( QKeySequence::Redo );
  m_undoAction->setEnabled ( false );
  m_redoAction->setEnabled ( false );

  m_drawMenu = new Qadra::Ui::DrawMenu ( this );
  menuBar ()->addMenu ( m_drawMenu );

  m_quickToolBar = new Qadra::Ui::QuickToolBar ( this );
  m_quickToolBar->addQuickAction ( m_undoAction );
  m_quickToolBar->addQuickAction ( m_redoAction );
  addToolBar ( Qt::TopToolBarArea, m_quickToolBar );

  m_drawToolBar = new Qadra::Ui::DrawToolBar ( this );
  addToolBar ( Qt::LeftToolBarArea, m_drawToolBar );

  m_canvas = new Qadra::Ui::Canvas ( this );
  setCentralWidget ( m_canvas );

  m_cmdLine = new Qadra::Ui::CmdLine ( this );
  addDockWidget ( Qt::BottomDockWidgetArea, m_cmdLine );

  connect ( m_drawMenu, &Qadra::Ui::DrawMenu::toolSelected, this, &Window::selectTool );
  connect ( m_drawToolBar, &Qadra::Ui::DrawToolBar::toolSelected, this, &Window::selectTool );
  connect ( m_canvas, &Qadra::Ui::Canvas::commandViewChanged, this, &Window::syncCommandUi );
  connect ( m_cmdLine, &Qadra::Ui::CmdLine::inputEdited, m_canvas,
            &Qadra::Ui::Canvas::setCommandInput );
  connect ( m_cmdLine, &Qadra::Ui::CmdLine::submitRequested, m_canvas,
            &Qadra::Ui::Canvas::submitCommandInput );
  connect ( m_undoAction, &QAction::triggered, m_canvas, &Qadra::Ui::Canvas::undo );
  connect ( m_redoAction, &QAction::triggered, m_canvas, &Qadra::Ui::Canvas::redo );

  if ( QUndoStack *undoStack = m_canvas->undoStack () )
  {
    connect ( undoStack, &QUndoStack::canUndoChanged, m_undoAction, &QAction::setEnabled );
    connect ( undoStack, &QUndoStack::canRedoChanged, m_redoAction, &QAction::setEnabled );
    connect ( undoStack, &QUndoStack::undoTextChanged, this, &Window::updateUndoActionText );
    connect ( undoStack, &QUndoStack::redoTextChanged, this, &Window::updateRedoActionText );

    m_undoAction->setEnabled ( undoStack->canUndo () );
    m_redoAction->setEnabled ( undoStack->canRedo () );
    updateUndoActionText ( undoStack->undoText () );
    updateRedoActionText ( undoStack->redoText () );
  }

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
  m_drawMenu->setSelectedToolKind ( m_canvas->activeToolKind () );
  m_drawToolBar->setSelectedToolKind ( m_canvas->activeToolKind () );
  m_cmdLine->render ( m_canvas->commandView () );
}

void Window::updateUndoActionText ( const QString &text )
{
  m_undoAction->setText ( text.isEmpty () ? tr ( "&Undo" ) : tr ( "&Undo %1" ).arg ( text ) );
}

void Window::updateRedoActionText ( const QString &text )
{
  m_redoAction->setText ( text.isEmpty () ? tr ( "&Redo" ) : tr ( "&Redo %1" ).arg ( text ) );
}
