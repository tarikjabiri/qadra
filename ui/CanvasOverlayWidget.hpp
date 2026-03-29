#ifndef QADRA_UI_CANVAS_OVERLAY_WIDGET_HPP
#define QADRA_UI_CANVAS_OVERLAY_WIDGET_HPP

#include "CanvasCursorOverlay.hpp"

#include <QWidget>

namespace Qadra::Ui
{
  class CanvasOverlayWidget final : public QWidget
  {
  public:
    explicit CanvasOverlayWidget ( QWidget *parent = nullptr );

    void setCursorState ( const CanvasCursorOverlay::State &state );

  protected:
    void paintEvent ( QPaintEvent *event ) override;

  private:
    CanvasCursorOverlay m_cursorOverlay;
    CanvasCursorOverlay::State m_state;
  };
} // namespace Qadra::Ui

#endif // QADRA_UI_CANVAS_OVERLAY_WIDGET_HPP
