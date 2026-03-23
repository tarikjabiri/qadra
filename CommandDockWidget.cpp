#include "CommandDockWidget.hpp"

#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QWidget>

namespace Qadra::Ui {
  CommandDockWidget::CommandDockWidget(const QString &title, QWidget *parent)
    : QDockWidget(title, parent) {
    auto *container = new QWidget(this);
    auto *layout = new QVBoxLayout(container);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);

    m_historyView = new QPlainTextEdit(container);
    m_historyView->setReadOnly(true);
    m_historyView->setMinimumHeight(100);
    layout->addWidget(m_historyView);

    m_commandLine = new QLineEdit(container);
    m_commandLine->setPlaceholderText(tr("Type a command"));
    layout->addWidget(m_commandLine);

    setWidget(container);

    connect(m_commandLine, &QLineEdit::returnPressed, this, [this]() {
      const QString commandText = m_commandLine->text().trimmed();
      if (commandText.isEmpty()) {
        return;
      }

      emit commandSubmitted(commandText);
      m_commandLine->clear();
    });
  }

  void CommandDockWidget::appendPrompt(const QString &prompt) const {
    if (!prompt.trimmed().isEmpty()) {
      m_historyView->appendPlainText(prompt);
    }
  }
} // Qadra::Ui
