#ifndef QADRA_UI_WINDOW_HPP
#define QADRA_UI_WINDOW_HPP

#include "tool/ToolKind.hpp"

#include <QMainWindow>

namespace Qadra::Ui
{
  class Canvas;
  class CmdLine;
  class DrawMenu;
  class DrawToolBar;
} // namespace Qadra::Ui

class Window : public QMainWindow
{
  Q_OBJECT

public:
  explicit Window ( QWidget *parent = nullptr );

  ~Window () override;

private:
  void selectTool ( Qadra::Tool::ToolKind kind );
  void syncCommandUi ();

  Qadra::Ui::Canvas *m_canvas{};
  Qadra::Ui::CmdLine *m_cmdLine{};
  Qadra::Ui::DrawMenu *m_drawMenu{};
  Qadra::Ui::DrawToolBar *m_drawToolBar{};
};

#endif // QADRA_UI_WINDOW_HPP
