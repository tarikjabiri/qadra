#include "CmdLine.hpp"

#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

namespace Qadra::Ui
{
  CmdLine::CmdLine ( QWidget *parent ) : QDockWidget ( parent )
  {
    setObjectName ( "CmdLine" );
    setAllowedAreas ( Qt::BottomDockWidgetArea );
    setTitleBarWidget ( new QWidget ( this ) );

    auto *content = new QWidget ( this );
    auto *layout = new QVBoxLayout ( content );

    layout->setContentsMargins ( 0, 0, 0, 0 );
    layout->setSpacing ( 0 );

    m_output = new QTextEdit ( content );
    m_output->setReadOnly ( true );
    m_output->setPlaceholderText ( tr ( "Command output" ) );

    m_input = new QLineEdit ( content );
    m_input->setPlaceholderText ( tr ( "Enter command" ) );

    layout->addWidget ( m_output );
    layout->addWidget ( m_input );

    setWidget ( content );
  }

  CmdLine::~CmdLine () = default;
} // namespace Qadra::Ui
