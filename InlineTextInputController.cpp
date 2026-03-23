#include "InlineTextInputController.hpp"

#include <algorithm>

#include <QFocusEvent>
#include <QInputMethodEvent>
#include <QKeyEvent>
#include <QVariant>

namespace Qadra::Ui {
  InlineTextInputController::InlineTextInputController(QWidget *parent)
    : QLineEdit(parent) {
    setFrame(false);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_InputMethodEnabled, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setStyleSheet("QLineEdit { background: transparent; color: transparent; border: none; selection-background-color: transparent; }");

    QObject::connect(this, &QLineEdit::textChanged, this, &InlineTextInputController::visualStateChanged);
    QObject::connect(this, &QLineEdit::cursorPositionChanged, this, [this](int, int) {
      emit visualStateChanged();
    });
    QObject::connect(this, &QLineEdit::selectionChanged, this, &InlineTextInputController::visualStateChanged);
  }

  void InlineTextInputController::suppressCallbacks(const bool suppress) noexcept {
    m_suppressCallbacks = suppress;
  }

  void InlineTextInputController::setInputCursorRectangleLogical(const QRectF &cursorRectangleLogical) noexcept {
    m_inputCursorRectangleLogical = cursorRectangleLogical;
  }

  void InlineTextInputController::paintEvent(QPaintEvent *) {
  }

  void InlineTextInputController::keyPressEvent(QKeyEvent *event) {
    if (!event) {
      return;
    }

    if (event->key() == Qt::Key_Escape) {
      if (!m_preeditText.isEmpty()) {
        QLineEdit::keyPressEvent(event);
        return;
      }
      if (!m_suppressCallbacks) {
        emit editingCancelled();
      }
      event->accept();
      return;
    }

    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
      if (!m_preeditText.isEmpty()) {
        QLineEdit::keyPressEvent(event);
        return;
      }
      if (!m_suppressCallbacks) {
        emit editingAccepted(text());
      }
      event->accept();
      return;
    }

    QLineEdit::keyPressEvent(event);
  }

  void InlineTextInputController::inputMethodEvent(QInputMethodEvent *event) {
    if (!event) {
      return;
    }

    m_preeditText = event->preeditString();
    m_preeditCursorPosition = static_cast<int>(m_preeditText.size());
    for (const QInputMethodEvent::Attribute &attribute: event->attributes()) {
      if (attribute.type == QInputMethodEvent::Cursor) {
        m_preeditCursorPosition = std::clamp(attribute.start, 0, static_cast<int>(m_preeditText.size()));
      }
    }

    QLineEdit::inputMethodEvent(event);
    emit visualStateChanged();
  }

  QVariant InlineTextInputController::inputMethodQuery(const Qt::InputMethodQuery query) const {
    if (query == Qt::ImCursorRectangle) {
      return m_inputCursorRectangleLogical;
    }

    return QLineEdit::inputMethodQuery(query);
  }

  void InlineTextInputController::focusOutEvent(QFocusEvent *event) {
    QLineEdit::focusOutEvent(event);
    emit visualStateChanged();
  }
} // Qadra::Ui
