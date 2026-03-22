#include "Window.hpp"
#include "Canvas.hpp"

#include <QIcon>

Window::Window(QWidget *parent)
  : QMainWindow(parent) {
  setWindowTitle("Qadra");
  setWindowIcon(QIcon(":/icons/qadra-icon.svg"));
  resize(800, 600);
  const auto canvas = new Qadra::Ui::Canvas();
  setCentralWidget(createWindowContainer(canvas, this));
}

Window::~Window() = default;
