#ifndef QADRA_UI_COMMANDDOCKWIDGET_HPP
#define QADRA_UI_COMMANDDOCKWIDGET_HPP

#include <QDockWidget>

class QLineEdit;
class QPlainTextEdit;

namespace Qadra::Ui {
  class CommandDockWidget : public QDockWidget {
    Q_OBJECT

  public:
    explicit CommandDockWidget(const QString &title, QWidget *parent = nullptr);

    void appendPrompt(const QString &prompt) const;

  signals:
    void commandSubmitted(const QString &commandText);

  private:
    QPlainTextEdit *m_historyView{};
    QLineEdit *m_commandLine{};
  };
} // Qadra::Ui

#endif // QADRA_UI_COMMANDDOCKWIDGET_HPP
