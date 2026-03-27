#ifndef QADRA_UI_CANVAS_CURSOR_OVERLAY_HPP
#define QADRA_UI_CANVAS_CURSOR_OVERLAY_HPP

#include <QPointF>

class QPainter;

namespace Qadra::Ui
{
  class CanvasCursorOverlay
  {
  public:
    struct State
    {
      QPointF position;
      bool visible = false;
      bool showPickbox = true;
    };

    void paint ( QPainter &painter, const State &state ) const;

  private:
    static void drawLinePair ( QPainter &painter, const QPointF &start, const QPointF &end );
    static void drawRectPair ( QPainter &painter, qreal left, qreal top, qreal size );

    static constexpr qreal kCrosshairHalfExtent = 52.0;
    static constexpr qreal kPickboxHalfExtent = 5.0;
    static constexpr qreal kPickboxSize = kPickboxHalfExtent * 2.0;
    static constexpr qreal kGap = 2.0;
  };
} // namespace Qadra::Ui

#endif // QADRA_UI_CANVAS_CURSOR_OVERLAY_HPP
