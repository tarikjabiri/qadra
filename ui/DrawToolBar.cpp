#include "DrawToolBar.hpp"

#include <QAction>
#include <QActionGroup>
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

    m_actionGroup = new QActionGroup ( this );
    m_actionGroup->setExclusionPolicy ( QActionGroup::ExclusionPolicy::ExclusiveOptional );

    m_lineAction = addAction ( QIcon ( ":/assets/draw-line.svg" ), tr ( "Line" ) );
    m_arcAction = addAction ( QIcon ( ":/assets/draw-arc.svg" ), tr ( "Arc" ) );
    m_textAction = addAction ( QIcon ( ":/assets/draw-text.svg" ), tr ( "Text" ) );

    m_lineAction->setCheckable ( true );
    m_arcAction->setCheckable ( true );
    m_textAction->setCheckable ( true );

    m_actionGroup->addAction ( m_lineAction );
    m_actionGroup->addAction ( m_arcAction );
    m_actionGroup->addAction ( m_textAction );

    m_arcAction->setEnabled ( false );
    m_textAction->setEnabled ( false );

    auto bindAction = [this] ( QAction *action, const Tool::ToolKind kind )
    {
      connect ( action, &QAction::toggled, this, [this, kind] ( const bool checked )
                { emit toolSelected ( checked ? kind : Tool::ToolKind::None ); } );
    };

    bindAction ( m_lineAction, Tool::ToolKind::Line );
    bindAction ( m_arcAction, Tool::ToolKind::Arc );
    bindAction ( m_textAction, Tool::ToolKind::Text );
  }

  DrawToolBar::~DrawToolBar () = default;

  Tool::ToolKind DrawToolBar::selectedToolKind () const noexcept
  {
    if ( m_lineAction && m_lineAction->isChecked () ) return Tool::ToolKind::Line;
    return Tool::ToolKind::None;
  }
} // namespace Qadra::Ui
