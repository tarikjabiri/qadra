#include "CanvasOverlayWidget.hpp"

#include <QPaintEvent>
#include <QPainter>

namespace Qadra::Ui
{
  CanvasOverlayWidget::CanvasOverlayWidget ( QWidget *parent ) : QWidget ( parent )
  {
    setAttribute ( Qt::WA_TransparentForMouseEvents );
    setAttribute ( Qt::WA_NoSystemBackground );
    setAttribute ( Qt::WA_TranslucentBackground );
    setFocusPolicy ( Qt::NoFocus );
  }

  void CanvasOverlayWidget::setCursorState ( const CanvasCursorOverlay::State &state )
  {
    if ( m_state.position == state.position && m_state.visible == state.visible &&
         m_state.showPickbox == state.showPickbox )
      return;

    m_state = state;
    update ();
  }

  void CanvasOverlayWidget::paintEvent ( QPaintEvent *event )
  {
    static_cast<void> ( event );

    QPainter painter ( this );
    m_cursorOverlay.paint ( painter, m_state );
  }
} // namespace Qadra::Ui
