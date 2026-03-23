#ifndef QADRA_UI_TEXTCREATECOMMAND_HPP
#define QADRA_UI_TEXTCREATECOMMAND_HPP

#include <optional>

#include <QColor>

#include "Command.hpp"

namespace Qadra::Ui {
  class TextCreateCommand final : public Command {
  public:
    TextCreateCommand() = default;

    [[nodiscard]] QString id() const override;

    void begin(CommandContext &context) override;

    void cancel(CommandContext &context) override;

    void onContextChanged(CommandContext &context) override;

    void onMousePress(const glm::dvec2 &worldPosition, Qt::MouseButton button, CommandContext &context) override;

    [[nodiscard]] bool isFinished() const override { return m_finished; }

    [[nodiscard]] const Qadra::Core::TextEntityData *previewTextEntity() const override;

  private:
    void cacheContext(CommandContext &context);

    [[nodiscard]] bool hasActiveContext() const;

    void startInlineEditing();

    void updatePreview();

    void acceptInlineEditing(const QString &text);

    void cancelInlineEditing(bool emitPrompt);

    [[nodiscard]] glm::vec4 currentColor() const;

    std::optional<glm::dvec2> m_insertionPoint;
    std::optional<Qadra::Core::TextEntityData> m_previewTextEntity;
    double m_heightWorld{32.0};
    double m_rotationDegrees{};
    QString m_content;
    QString m_fontKey;
    QColor m_color{227, 237, 255, 242};
    bool m_finished{false};
    Qadra::Core::Document *m_document{};
    Canvas *m_canvas{};
    std::function<void()> m_requestRedraw;
    std::function<void(const QString &prompt)> m_appendPrompt;
    std::function<void()> m_notifyStateChanged;
  };
} // Qadra::Ui

#endif // QADRA_UI_TEXTCREATECOMMAND_HPP
