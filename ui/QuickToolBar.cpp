#include "QuickToolBar.hpp"

#include <QAction>
#include <QSize>

namespace Qadra::Ui
{
  QuickToolBar::QuickToolBar ( QWidget *parent ) : QToolBar ( parent )
  {
    setObjectName ( "QuickToolBar" );
    setWindowTitle ( tr ( "Quick" ) );
    setToolButtonStyle ( Qt::ToolButtonIconOnly );
    setIconSize ( QSize ( 32, 32 ) );
    setStyleSheet ( "border-bottom: none;" );
  }

  QuickToolBar::~QuickToolBar () = default;

  void QuickToolBar::addQuickAction ( QAction *action )
  {
    if ( ! action ) return;
    addAction ( action );
  }

  void QuickToolBar::addQuickSeparator () { addSeparator (); }
} // namespace Qadra::Ui
