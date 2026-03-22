#ifndef QADRA_UI_CANVAS_HPP
#define QADRA_UI_CANVAS_HPP

#include <QWindow>

#include "Grid.hpp"
#include "Program.hpp"
#include "VertexArray.hpp"
#include "Camera.hpp"
#include "GridPass.hpp"

namespace Qadra::Ui {
  class Canvas : public QWindow {
    Q_OBJECT

  public:
    explicit Canvas();

    static QString loadShaderSource(const QString& filename);

  protected:
    void exposeEvent(QExposeEvent *) override;

    bool event(QEvent *) override;

  private:
    void initialize();

    void render();

    static QFunctionPointer getProcAddress(const char *procName);

  protected:
    void mousePressEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

    void resizeEvent(QResizeEvent *event) override;

  private:
    QOpenGLContext *m_context = nullptr;
    bool m_initialized = false;

    Core::Camera m_camera;
    QPointF m_lastMousePosition;
    bool m_panning{false};

    std::optional<GL::VertexArray> m_vertexArray;
    std::optional<GL::Buffer> m_buffer;
    std::optional<GL::Program> m_program;
    std::optional<GL::Program> m_programGrid;
    std::optional<Core::Grid> m_grid;
    std::optional<Render::GridPass> m_gridPass;
  };
} // Qadra::Ui

#endif // QADRA_UI_CANVAS_HPP
