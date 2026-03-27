#include "CmdLine.hpp"

#include <QFont>
#include <QFontDatabase>
#include <QLineEdit>
#include <QScrollBar>
#include <QSignalBlocker>
#include <QStringList>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

namespace
{
  [[nodiscard]] QFont makeCommandFont ()
  {
    QFont font = QFontDatabase::systemFont ( QFontDatabase::FixedFont );
    font.setStyleHint ( QFont::Monospace );
    font.setFixedPitch ( true );

    const QStringList preferredFamilies = {
        "Cascadia Mono",
        "Cascadia Code",
        "Consolas",
        "Lucida Console",
    };

    const QFontDatabase database;
    for ( const QString &family : preferredFamilies )
    {
      if ( database.families ().contains ( family ) )
      {
        font.setFamily ( family );
        break;
      }
    }

    return font;
  }

  [[nodiscard]] QString formatHistory ( const std::vector<Qadra::Command::HistoryEntry> &history )
  {
    QStringList lines;
    lines.reserve ( static_cast<qsizetype> ( history.size () ) );

    for ( const auto &entry : history )
    {
      QString text = QString::fromStdString ( entry.text );

      if ( entry.kind == Qadra::Command::HistoryEntryKind::Error )
        text = QObject::tr ( "Error: %1" ).arg ( text );

      lines.push_back ( text );
    }

    return lines.join ( '\n' );
  }
} // namespace

namespace Qadra::Ui
{
  CmdLine::CmdLine ( QWidget *parent ) : QDockWidget ( parent )
  {
    setObjectName ( "CmdLine" );
    setAllowedAreas ( Qt::BottomDockWidgetArea );
    setTitleBarWidget ( new QWidget ( this ) );

    auto *content = new QWidget ( this );
    auto *layout = new QVBoxLayout ( content );
    const QFont commandFont = makeCommandFont ();

    layout->setContentsMargins ( 0, 0, 0, 0 );
    layout->setSpacing ( 0 );

    m_output = new QTextEdit ( content );
    m_output->setReadOnly ( true );
    m_output->setPlaceholderText ( tr ( "Command output" ) );
    m_output->setFont ( commandFont );

    m_input = new QLineEdit ( content );
    m_input->setPlaceholderText ( tr ( "Enter command" ) );
    m_input->setFont ( commandFont );

    layout->addWidget ( m_output );
    layout->addWidget ( m_input );

    setWidget ( content );

    connect ( m_input, &QLineEdit::textEdited, this, &CmdLine::inputEdited );
    connect ( m_input, &QLineEdit::returnPressed, this, &CmdLine::submitRequested );

    auto *scrollBar = m_output->verticalScrollBar ();
    connect ( scrollBar, &QScrollBar::valueChanged, this,
              [this] ( const int ) { m_followOutput = isOutputAtBottom (); } );
    connect ( scrollBar, &QScrollBar::rangeChanged, this,
              [this] ( const int, const int )
              {
                if ( m_followOutput ) scrollOutputToBottom ();
              } );
  }

  CmdLine::~CmdLine () = default;

  void CmdLine::render ( const Command::View &view )
  {
    const QSignalBlocker blocker ( m_input );

    const QString prompt = QString::fromStdString ( view.prompt );
    const QString input = QString::fromStdString ( view.input );
    const QString history = formatHistory ( view.history );
    const bool historyChanged = m_output->toPlainText () != history;
    const bool shouldStickToBottom = historyChanged && m_followOutput;
    const int previousScrollValue = m_output->verticalScrollBar ()->value ();

    m_input->setPlaceholderText ( prompt.isEmpty () ? tr ( "Enter command" ) : prompt );
    if ( m_input->text () != input ) m_input->setText ( input );
    if ( historyChanged )
    {
      m_output->setPlainText ( history );

      if ( shouldStickToBottom ) scrollOutputToBottom ();
      else
        m_output->verticalScrollBar ()->setValue ( previousScrollValue );
    }
  }

  bool CmdLine::isOutputAtBottom () const
  {
    const QScrollBar *scrollBar = m_output->verticalScrollBar ();
    return scrollBar->value () >= scrollBar->maximum ();
  }

  void CmdLine::scrollOutputToBottom ()
  {
    QScrollBar *scrollBar = m_output->verticalScrollBar ();
    scrollBar->setValue ( scrollBar->maximum () );
  }
} // namespace Qadra::Ui
