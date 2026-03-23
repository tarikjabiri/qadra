#ifndef QADRA_UI_COMMANDMANAGER_HPP
#define QADRA_UI_COMMANDMANAGER_HPP

#include <functional>
#include <memory>

#include <QHash>
#include <QObject>

#include "Command.hpp"

namespace Qadra::Core {
  class Document;
}

namespace Qadra::Ui {
  class Canvas;

  class CommandManager : public QObject {
    Q_OBJECT

  public:
    using Factory = std::function<std::unique_ptr<Command>()>;

    CommandManager(Qadra::Core::Document &document, Canvas &canvas, QObject *parent = nullptr);

    void registerCommand(const QString &name, Factory factory);

    void start(const QString &name);

    void cancelActiveCommand();

    void refreshActiveCommand();

    void handleMouseMove(const glm::dvec2 &worldPosition);

    void handleMousePress(const glm::dvec2 &worldPosition, Qt::MouseButton button);

    void handleKeyPress(QKeyEvent *event);

    [[nodiscard]] Command *activeCommand() const noexcept { return m_activeCommand.get(); }

    [[nodiscard]] const Qadra::Core::TextEntityData *previewTextEntity() const noexcept;

  signals:
    void activeCommandChanged(Qadra::Ui::Command *command);

    void promptEmitted(const QString &prompt);

  private:
    [[nodiscard]] CommandContext makeContext();

    template <typename Invoker>
    void dispatchActiveCommand(Invoker &&invoker);

    void finalizeIfFinished();

    void clearActiveCommand();

    Qadra::Core::Document &m_document;
    Canvas &m_canvas;
    QHash<QString, Factory> m_factories;
    std::unique_ptr<Command> m_activeCommand;
  };
} // Qadra::Ui

#endif // QADRA_UI_COMMANDMANAGER_HPP
