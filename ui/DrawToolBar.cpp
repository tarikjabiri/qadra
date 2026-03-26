#include "DrawToolBar.hpp"

#include <QAction>
#include <QIcon>
#include <QSize>

namespace Qadra::Ui
{
  DrawToolBar::DrawToolBar ( QWidget *parent ) : QToolBar ( parent )
  {
    setObjectName ( "DrawToolBar" );
    setWindowTitle ( tr ( "Draw" ) );
    setMovable ( false );
    setFloatable ( false );
    setStyleSheet ( "border-bottom: none;" );

    setToolButtonStyle ( Qt::ToolButtonIconOnly );
    setIconSize ( QSize ( 32, 32 ) );

    m_lineAction = addAction ( QIcon ( ":/assets/draw-line.svg" ), tr ( "Line" ) );
    m_arcAction = addAction ( QIcon ( ":/assets/draw-arc.svg" ), tr ( "Arc" ) );
    m_textAction = addAction ( QIcon ( ":/assets/draw-text.svg" ), tr ( "Text" ) );
  }

  DrawToolBar::~DrawToolBar () = default;
} // namespace Qadra::Ui
