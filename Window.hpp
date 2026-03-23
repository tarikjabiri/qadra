#ifndef QADRA_UI_WINDOW_HPP
#define QADRA_UI_WINDOW_HPP

#include <QMainWindow>

#include "Document.hpp"

class QAction;
class QShortcut;

namespace Qadra::Ui {
  class Canvas;
  class CommandDockWidget;
  class CommandManager;
}

class Window : public QMainWindow {
  Q_OBJECT

public:
  explicit Window(QWidget *parent = nullptr);

  ~Window() override;

private:
  Qadra::Core::Document m_document;
  Qadra::Ui::Canvas *m_canvas{};
  Qadra::Ui::CommandManager *m_commandManager{};
  Qadra::Ui::CommandDockWidget *m_commandDockWidget{};
  QAction *m_textAction{};
  QAction *m_showDebugTextSceneAction{};
  QShortcut *m_cancelShortcut{};
};

#endif // QADRA_UI_WINDOW_HPP
