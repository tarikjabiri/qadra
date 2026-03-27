#ifndef QADRA_UI_WINDOW_HPP
#define QADRA_UI_WINDOW_HPP

#include "tool/ToolKind.hpp"

#include <QMainWindow>
#include <QString>

namespace Qadra::Ui
{
  class Canvas;
  class CmdLine;
  class DrawMenu;
  class DrawToolBar;
  class QuickToolBar;
} // namespace Qadra::Ui

class QAction;
class QMenu;

class Window : public QMainWindow
{
  Q_OBJECT

public:
  explicit Window ( QWidget *parent = nullptr );

  ~Window () override;

private:
  void selectTool ( Qadra::Tool::ToolKind kind );
  void syncCommandUi ();
  void updateUndoActionText ( const QString &text );
  void updateRedoActionText ( const QString &text );

  Qadra::Ui::Canvas *m_canvas{};
  Qadra::Ui::CmdLine *m_cmdLine{};
  QMenu *m_editMenu{};
  Qadra::Ui::DrawMenu *m_drawMenu{};
  Qadra::Ui::DrawToolBar *m_drawToolBar{};
  Qadra::Ui::QuickToolBar *m_quickToolBar{};
  QAction *m_undoAction{};
  QAction *m_redoAction{};
};

#endif // QADRA_UI_WINDOW_HPP
