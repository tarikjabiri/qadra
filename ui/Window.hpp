#ifndef QADRA_UI_WINDOW_HPP
#define QADRA_UI_WINDOW_HPP

#include <QMainWindow>

namespace Qadra::Ui
{
  class Canvas;
  class CmdLine;
} // namespace Qadra::Ui

class Window : public QMainWindow
{
  Q_OBJECT

public:
  explicit Window ( QWidget *parent = nullptr );

  ~Window () override;

private:
  Qadra::Ui::Canvas *m_canvas{};
  Qadra::Ui::CmdLine *m_cmdLine{};
};

#endif // QADRA_UI_WINDOW_HPP
