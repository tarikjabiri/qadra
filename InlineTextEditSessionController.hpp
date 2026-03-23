#ifndef QADRA_UI_INLINETEXTEDITSESSIONCONTROLLER_HPP
#define QADRA_UI_INLINETEXTEDITSESSIONCONTROLLER_HPP

#include <functional>
#include <optional>

#include <QObject>
#include <QPointer>
#include <QRectF>
#include <QString>
#include <QTimer>

#include <glm/glm.hpp>

#include "TextLayout.hpp"

class QWidget;

namespace Qadra::Ui {
  class InlineTextInputController;

  class InlineTextEditSessionController : public QObject {
    Q_OBJECT

  public:
    struct Request {
      glm::dvec2 worldPosition{0.0};
      double heightWorld{32.0};
      double rotationRadians{0.0};
      QString text;
      QString fontKey;
      glm::vec4 color{1.0f};
      Qadra::Core::TextLayoutBounds stableLineBoundsLocal{};
    };

    struct Callbacks {
      std::function<void(const QString &text)> onTextChanged;
      std::function<void(const QString &text)> onAccepted;
      std::function<void()> onCancelled;
    };

    struct Snapshot {
      glm::dvec2 worldPosition{0.0};
      double heightWorld{32.0};
      double rotationRadians{0.0};
      QString fontKey;
      glm::vec4 color{1.0f};
      Qadra::Core::TextLayoutBounds stableLineBoundsLocal{};
      QString displayText;
      int cursorIndex{};
      int selectionStart{};
      int selectionEnd{};
      bool caretVisible{true};
    };

    explicit InlineTextEditSessionController(QWidget &hostWidget, QObject *parent = nullptr);

    void begin(const Request &request, Callbacks callbacks);

    void update(const Request &request);

    void end();

    [[nodiscard]] bool hasActiveSession() const noexcept;

    [[nodiscard]] std::optional<Snapshot> snapshot() const;

    void setInputCursorRectangleLogical(const QRectF &cursorRectangleLogical);

    void focus(Qt::FocusReason focusReason);

    void setSelecting(bool selecting) noexcept { m_selecting = selecting; }

    [[nodiscard]] bool selecting() const noexcept { return m_selecting; }

    void setSelectionAnchor(int selectionAnchor) noexcept { m_selectionAnchor = selectionAnchor; }

    [[nodiscard]] int selectionAnchor() const noexcept { return m_selectionAnchor; }

    [[nodiscard]] bool hasPreeditText() const noexcept;

    [[nodiscard]] int cursorPosition() const noexcept;

    void setCursorPosition(int cursorPosition);

    void setSelection(int selectionStart, int selectionLength);

    [[nodiscard]] bool hasSelectedText() const noexcept;

    [[nodiscard]] int selectionStart() const noexcept;

    [[nodiscard]] QString selectedText() const;

    [[nodiscard]] QString committedText() const;

  signals:
    void stateChanged();

  private:
    [[nodiscard]] InlineTextInputController *inputController() const noexcept;

    [[nodiscard]] QString displayText() const;

    [[nodiscard]] int displayCursorIndex() const noexcept;

    [[nodiscard]] int selectionEnd() const noexcept;

    void connectInputController();

    void restartCaretBlink();

    void stopCaretBlink();

    QWidget &m_hostWidget;
    Request m_request;
    Callbacks m_callbacks;
    QPointer<InlineTextInputController> m_inputController;
    QTimer m_caretBlinkTimer;
    QRectF m_inputCursorRectangleLogical;
    bool m_caretVisible{true};
    bool m_selecting{false};
    int m_selectionAnchor{};
  };
} // Qadra::Ui

#endif // QADRA_UI_INLINETEXTEDITSESSIONCONTROLLER_HPP
