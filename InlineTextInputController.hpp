#ifndef QADRA_UI_INLINETEXTINPUTCONTROLLER_HPP
#define QADRA_UI_INLINETEXTINPUTCONTROLLER_HPP

#include <QLineEdit>
#include <QRectF>
#include <QVariant>

class QInputMethodEvent;
class QPaintEvent;
class QFocusEvent;

namespace Qadra::Ui {
  class InlineTextInputController : public QLineEdit {
    Q_OBJECT

  public:
    explicit InlineTextInputController(QWidget *parent = nullptr);

    void suppressCallbacks(bool suppress) noexcept;

    void setInputCursorRectangleLogical(const QRectF &cursorRectangleLogical) noexcept;

    [[nodiscard]] const QString &preeditText() const noexcept { return m_preeditText; }

    [[nodiscard]] int preeditCursorPosition() const noexcept { return m_preeditCursorPosition; }

  signals:
    void editingAccepted(const QString &text);

    void editingCancelled();

    void visualStateChanged();

  protected:
    void paintEvent(QPaintEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

    void inputMethodEvent(QInputMethodEvent *event) override;

    QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;

    void focusOutEvent(QFocusEvent *event) override;

  private:
    QRectF m_inputCursorRectangleLogical;
    QString m_preeditText;
    int m_preeditCursorPosition{};
    bool m_suppressCallbacks{false};
  };
} // Qadra::Ui

#endif // QADRA_UI_INLINETEXTINPUTCONTROLLER_HPP
