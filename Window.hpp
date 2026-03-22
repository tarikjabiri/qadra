#ifndef QADRA_UI_WINDOW_HPP
#define QADRA_UI_WINDOW_HPP

#include <QMainWindow>

class Window : public QMainWindow {
  Q_OBJECT

public:
  explicit Window(QWidget *parent = nullptr);

  ~Window() override;
};


#endif // QADRA_UI_WINDOW_HPP
