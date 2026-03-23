#include "InlineTextEditSessionController.hpp"

#include <algorithm>
#include <utility>

#include "InlineTextInputController.hpp"

namespace {
  constexpr int inlineCaretBlinkIntervalMilliseconds = 530;
}

namespace Qadra::Ui {
  InlineTextEditSessionController::InlineTextEditSessionController(QWidget &hostWidget, QObject *parent)
    : QObject(parent),
      m_hostWidget(hostWidget) {
    m_caretBlinkTimer.setInterval(inlineCaretBlinkIntervalMilliseconds);
    QObject::connect(&m_caretBlinkTimer, &QTimer::timeout, this, [this]() {
      m_caretVisible = !m_caretVisible;
      emit stateChanged();
    });
  }

  void InlineTextEditSessionController::begin(const Request &request, Callbacks callbacks) {
    end();

    m_request = request;
    m_callbacks = std::move(callbacks);
    m_inputController = new InlineTextInputController(&m_hostWidget);
    m_inputController->setGeometry(0, 0, 1, 1);
    m_inputController->setText(request.text);
    m_inputController->setCursorPosition(static_cast<int>(request.text.size()));
    m_inputController->show();
    m_inputController->raise();
    m_inputController->setFocus(Qt::OtherFocusReason);
    m_inputController->setInputCursorRectangleLogical(m_inputCursorRectangleLogical);

    connectInputController();

    m_selecting = false;
    m_selectionAnchor = cursorPosition();
    restartCaretBlink();
    emit stateChanged();
  }

  void InlineTextEditSessionController::update(const Request &request) {
    if (!inputController()) {
      return;
    }

    m_request = request;
    if (m_inputController->text() != request.text) {
      const int clampedCursorPosition = std::clamp(cursorPosition(), 0, static_cast<int>(request.text.size()));
      m_inputController->setText(request.text);
      m_inputController->setCursorPosition(clampedCursorPosition);
    }
    m_inputController->setInputCursorRectangleLogical(m_inputCursorRectangleLogical);
    emit stateChanged();
  }

  void InlineTextEditSessionController::end() {
    if (!inputController()) {
      return;
    }

    m_inputController->suppressCallbacks(true);
    m_inputController->deleteLater();
    m_inputController = nullptr;
    m_request = {};
    m_callbacks = {};
    m_inputCursorRectangleLogical = {};
    m_selecting = false;
    m_selectionAnchor = 0;
    stopCaretBlink();
    emit stateChanged();
  }

  bool InlineTextEditSessionController::hasActiveSession() const noexcept {
    return m_inputController;
  }

  std::optional<InlineTextEditSessionController::Snapshot> InlineTextEditSessionController::snapshot() const {
    if (!inputController()) {
      return std::nullopt;
    }

    return Snapshot{
      .worldPosition = m_request.worldPosition,
      .heightWorld = m_request.heightWorld,
      .rotationRadians = m_request.rotationRadians,
      .fontKey = m_request.fontKey,
      .color = m_request.color,
      .stableLineBoundsLocal = m_request.stableLineBoundsLocal,
      .displayText = displayText(),
      .cursorIndex = displayCursorIndex(),
      .selectionStart = selectionStart(),
      .selectionEnd = selectionEnd(),
      .caretVisible = m_caretVisible
    };
  }

  void InlineTextEditSessionController::setInputCursorRectangleLogical(const QRectF &cursorRectangleLogical) {
    m_inputCursorRectangleLogical = cursorRectangleLogical;
    if (inputController()) {
      m_inputController->setInputCursorRectangleLogical(cursorRectangleLogical);
    }
  }

  void InlineTextEditSessionController::focus(const Qt::FocusReason focusReason) {
    if (inputController()) {
      m_inputController->setFocus(focusReason);
    }
  }

  bool InlineTextEditSessionController::hasPreeditText() const noexcept {
    return inputController() && !m_inputController->preeditText().isEmpty();
  }

  int InlineTextEditSessionController::cursorPosition() const noexcept {
    return inputController() ? m_inputController->cursorPosition() : 0;
  }

  void InlineTextEditSessionController::setCursorPosition(const int cursorPositionValue) {
    if (inputController()) {
      m_inputController->setCursorPosition(cursorPositionValue);
    }
  }

  void InlineTextEditSessionController::setSelection(const int selectionStartValue, const int selectionLength) {
    if (inputController()) {
      m_inputController->setSelection(selectionStartValue, selectionLength);
    }
  }

  bool InlineTextEditSessionController::hasSelectedText() const noexcept {
    return inputController() && m_inputController->hasSelectedText();
  }

  int InlineTextEditSessionController::selectionStart() const noexcept {
    if (!hasSelectedText()) {
      return 0;
    }

    const int currentSelectionStart = m_inputController->selectionStart();
    return currentSelectionStart >= 0 ? currentSelectionStart : 0;
  }

  QString InlineTextEditSessionController::selectedText() const {
    return inputController() ? m_inputController->selectedText() : QString{};
  }

  QString InlineTextEditSessionController::committedText() const {
    return inputController() ? m_inputController->text() : QString{};
  }

  InlineTextInputController *InlineTextEditSessionController::inputController() const noexcept {
    return m_inputController.data();
  }

  QString InlineTextEditSessionController::displayText() const {
    const QString currentCommittedText = committedText();
    if (!inputController() || m_inputController->preeditText().isEmpty()) {
      return currentCommittedText;
    }

    QString currentDisplayText = currentCommittedText;
    currentDisplayText.insert(m_inputController->cursorPosition(), m_inputController->preeditText());
    return currentDisplayText;
  }

  int InlineTextEditSessionController::displayCursorIndex() const noexcept {
    return inputController() ? m_inputController->cursorPosition() + m_inputController->preeditCursorPosition() : 0;
  }

  int InlineTextEditSessionController::selectionEnd() const noexcept {
    return selectionStart() + static_cast<int>(selectedText().size());
  }

  void InlineTextEditSessionController::connectInputController() {
    QObject::connect(m_inputController,
                     &QLineEdit::textChanged,
                     this,
                     [this](const QString &text) {
                       if (m_callbacks.onTextChanged) {
                         m_callbacks.onTextChanged(text);
                       }
                       restartCaretBlink();
                       emit stateChanged();
                     });
    QObject::connect(m_inputController,
                     &QLineEdit::cursorPositionChanged,
                     this,
                     [this](int, int) {
                       restartCaretBlink();
                       emit stateChanged();
                     });
    QObject::connect(m_inputController, &QLineEdit::selectionChanged, this, [this]() {
      restartCaretBlink();
      emit stateChanged();
    });
    QObject::connect(m_inputController, &InlineTextInputController::visualStateChanged, this, [this]() {
      restartCaretBlink();
      emit stateChanged();
    });
    QObject::connect(m_inputController,
                     &InlineTextInputController::editingAccepted,
                     this,
                     [this](const QString &text) {
                       if (m_callbacks.onAccepted) {
                         m_callbacks.onAccepted(text);
                       }
                     });
    QObject::connect(m_inputController, &InlineTextInputController::editingCancelled, this, [this]() {
      if (m_callbacks.onCancelled) {
        m_callbacks.onCancelled();
      }
    });
  }

  void InlineTextEditSessionController::restartCaretBlink() {
    m_caretVisible = true;
    if (!m_caretBlinkTimer.isActive()) {
      m_caretBlinkTimer.start();
    }
  }

  void InlineTextEditSessionController::stopCaretBlink() {
    m_caretBlinkTimer.stop();
    m_caretVisible = true;
  }
} // Qadra::Ui
