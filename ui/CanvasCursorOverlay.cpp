#include "CanvasCursorOverlay.hpp"

#include <QColor>
#include <QPainter>
#include <QPen>
#include <QRectF>

namespace
{
  QPen shadowPen ()
  {
    QPen pen ( QColor ( 12, 14, 18, 170 ) );
    pen.setCosmetic ( true );
    pen.setWidthF ( 3.0 );
    return pen;
  }

  QPen foregroundPen ()
  {
    QPen pen ( QColor ( 238, 241, 245, 235 ) );
    pen.setCosmetic ( true );
    pen.setWidthF ( 1.0 );
    return pen;
  }
} // namespace

namespace Qadra::Ui
{
  void CanvasCursorOverlay::paint ( QPainter &painter, const State &state ) const
  {
    if ( ! state.visible ) return;

    painter.save ();
    painter.setRenderHint ( QPainter::Antialiasing, false );

    const qreal x = state.position.x ();
    const qreal y = state.position.y ();

    if ( state.showPickbox )
    {
      drawLinePair ( painter, { x - kCrosshairHalfExtent, y },
                     { x - kPickboxHalfExtent - kGap, y } );
      drawLinePair ( painter, { x + kPickboxHalfExtent + kGap, y },
                     { x + kCrosshairHalfExtent, y } );
      drawLinePair ( painter, { x, y - kCrosshairHalfExtent },
                     { x, y - kPickboxHalfExtent - kGap } );
      drawLinePair ( painter, { x, y + kPickboxHalfExtent + kGap },
                     { x, y + kCrosshairHalfExtent } );
      drawRectPair ( painter, x - kPickboxHalfExtent, y - kPickboxHalfExtent, kPickboxSize );
    }
    else
    {
      drawLinePair ( painter, { x - kCrosshairHalfExtent, y }, { x + kCrosshairHalfExtent, y } );
      drawLinePair ( painter, { x, y - kCrosshairHalfExtent }, { x, y + kCrosshairHalfExtent } );
    }

    painter.restore ();
  }

  void CanvasCursorOverlay::drawLinePair ( QPainter &painter, const QPointF &start,
                                           const QPointF &end )
  {
    painter.setPen ( shadowPen () );
    painter.drawLine ( start, end );
    painter.setPen ( foregroundPen () );
    painter.drawLine ( start, end );
  }

  void CanvasCursorOverlay::drawRectPair ( QPainter &painter, const qreal left, const qreal top,
                                           const qreal size )
  {
    const QRectF rect ( left, top, size, size );
    painter.setPen ( shadowPen () );
    painter.drawRect ( rect );
    painter.setPen ( foregroundPen () );
    painter.drawRect ( rect );
  }
} // namespace Qadra::Ui
