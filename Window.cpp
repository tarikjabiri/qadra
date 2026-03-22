#include "Window.hpp"
#include "Canvas.hpp"

Window::Window(QWidget *parent)
  : QMainWindow(parent) {
  setWindowTitle("qadra");
  resize(800, 600);
  const auto canvas = new Qadra::Ui::Canvas();
  setCentralWidget(createWindowContainer(canvas, this));
}

Window::~Window() = default;
