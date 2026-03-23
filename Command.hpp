#ifndef QADRA_UI_COMMAND_HPP
#define QADRA_UI_COMMAND_HPP

#include <functional>

#include <QKeyEvent>
#include <QString>
#include "TextEntity.hpp"

namespace Qadra::Core {
  class Document;
}

namespace Qadra::Ui {
  class Canvas;

  struct CommandContext {
    Qadra::Core::Document &document;
    Canvas &canvas;
    std::function<void()> requestRedraw;
    std::function<void(const QString &prompt)> appendPrompt;
    std::function<void()> notifyStateChanged;
  };

  class Command {
  public:
    virtual ~Command() = default;

    [[nodiscard]] virtual QString id() const = 0;

    virtual void begin(CommandContext &context) = 0;

    virtual void cancel(CommandContext &context) = 0;

    virtual void onContextChanged(CommandContext &context) {}

    virtual void onMouseMove(const glm::dvec2 &worldPosition, CommandContext &context) {}

    virtual void onMousePress(const glm::dvec2 &worldPosition, Qt::MouseButton button, CommandContext &context) {}

    virtual void onKeyPress(QKeyEvent &event, CommandContext &context) {}

    [[nodiscard]] virtual bool isFinished() const = 0;

    [[nodiscard]] virtual const Qadra::Core::TextEntityData *previewTextEntity() const { return nullptr; }
  };
} // Qadra::Ui

#endif // QADRA_UI_COMMAND_HPP
