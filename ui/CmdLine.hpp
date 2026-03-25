#ifndef QADRA_UI_CMD_LINE_HPP
#define QADRA_UI_CMD_LINE_HPP

#include <QDockWidget>

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

  private:
    QTextEdit *m_output{};
    QLineEdit *m_input{};
  };
} // namespace Qadra::Ui

#endif // QADRA_UI_CMD_LINE_HPP
