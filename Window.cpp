#include "Window.hpp"

#include <memory>

#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QMenuBar>
#include <QShortcut>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

#include "Canvas.hpp"
#include "CommandDockWidget.hpp"
#include "CommandManager.hpp"
#include "TextCreateCommand.hpp"

Window::Window(QWidget *parent)
  : QMainWindow(parent) {
  setWindowTitle("Qadra");
  setWindowIcon(QIcon(":/icons/qadra-icon.svg"));
  resize(1200, 800);

  auto *centralHost = new QWidget(this);
  auto *centralLayout = new QVBoxLayout(centralHost);
  centralLayout->setContentsMargins(0, 0, 0, 0);
  centralLayout->setSpacing(0);

  m_canvas = new Qadra::Ui::Canvas(centralHost);
  m_commandManager = new Qadra::Ui::CommandManager(m_document, *m_canvas, this);
  m_commandDockWidget = new Qadra::Ui::CommandDockWidget(tr("Command"), this);

  m_canvas->setDocument(&m_document);
  m_canvas->setCommandManager(m_commandManager);

  centralLayout->addWidget(m_canvas);
  setCentralWidget(centralHost);

  addDockWidget(Qt::BottomDockWidgetArea, m_commandDockWidget);

  auto *drawToolBar = addToolBar(tr("Draw"));
  m_textAction = drawToolBar->addAction(tr("TEXT"));

  auto *viewMenu = menuBar()->addMenu(tr("View"));
  m_showDebugTextSceneAction = viewMenu->addAction(tr("Show Debug Text Scene"));
  m_showDebugTextSceneAction->setCheckable(true);

  m_cancelShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);

  m_commandManager->registerCommand("TEXT", []() {
    return std::make_unique<Qadra::Ui::TextCreateCommand>();
  });

  connect(m_commandDockWidget,
          &Qadra::Ui::CommandDockWidget::commandSubmitted,
          m_commandManager,
          &Qadra::Ui::CommandManager::start);

  connect(m_commandManager,
          &Qadra::Ui::CommandManager::promptEmitted,
          m_commandDockWidget,
          &Qadra::Ui::CommandDockWidget::appendPrompt);

  connect(m_canvas,
          &Qadra::Ui::Canvas::fontsChanged,
          m_commandManager,
          &Qadra::Ui::CommandManager::refreshActiveCommand);

  connect(m_textAction, &QAction::triggered, this, [this]() {
    m_commandManager->start("TEXT");
  });

  connect(m_showDebugTextSceneAction, &QAction::toggled, m_canvas, &Qadra::Ui::Canvas::setDebugTextOverlayEnabled);

  connect(m_cancelShortcut, &QShortcut::activated, this, [this]() {
    m_commandManager->cancelActiveCommand();
  });

  m_commandDockWidget->appendPrompt("Ready. Type TEXT in the command line or use the toolbar.");
}

Window::~Window() = default;
