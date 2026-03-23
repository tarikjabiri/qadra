#ifndef QADRA_UI_WINDOW_HPP
#define QADRA_UI_WINDOW_HPP

#include <QMainWindow>

class QAction;

namespace Qadra::Ui
{
  class Canvas;
} // namespace Qadra::Ui

class Window : public QMainWindow
{
  Q_OBJECT

public:
  explicit Window ( QWidget *parent = nullptr );

  ~Window () override;

private:
  Qadra::Ui::Canvas *m_canvas{};
};

#endif // QADRA_UI_WINDOW_HPP
