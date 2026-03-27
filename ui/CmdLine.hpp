#ifndef QADRA_UI_CMD_LINE_HPP
#define QADRA_UI_CMD_LINE_HPP

#include "command/View.hpp"

#include <QDockWidget>
#include <QString>

class QTextEdit;
class QLineEdit;

namespace Qadra::Ui
{
  class CmdLine : public QDockWidget
  {
    Q_OBJECT

  public:
    explicit CmdLine ( QWidget *parent = nullptr );

    ~CmdLine () override;

    void render ( const Qadra::Command::View &view );

  signals:
    void inputEdited ( const QString &text );
    void submitRequested ();

  private:
    [[nodiscard]] bool isOutputAtBottom () const;

    void scrollOutputToBottom ();

    QTextEdit *m_output{};
    QLineEdit *m_input{};
    bool m_followOutput = true;
  };
} // namespace Qadra::Ui

#endif // QADRA_UI_CMD_LINE_HPP
