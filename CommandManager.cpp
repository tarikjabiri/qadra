#include "CommandManager.hpp"

#include <utility>

#include "Canvas.hpp"
#include "Document.hpp"

namespace Qadra::Ui {
  CommandManager::CommandManager(Qadra::Core::Document &document, Canvas &canvas, QObject *parent)
    : QObject(parent),
      m_document(document),
      m_canvas(canvas) {
  }

  void CommandManager::registerCommand(const QString &name, Factory factory) {
    m_factories.insert(name.trimmed().toUpper(), std::move(factory));
  }

  void CommandManager::start(const QString &name) {
    const QString normalizedName = name.trimmed().toUpper();
    if (normalizedName.isEmpty()) {
      return;
    }

    const auto iterator = m_factories.constFind(normalizedName);
    if (iterator == m_factories.constEnd()) {
      emit promptEmitted(QString("Unknown command: %1").arg(normalizedName));
      return;
    }

    if (m_activeCommand) {
      cancelActiveCommand();
    }

    m_activeCommand = iterator.value()();
    CommandContext context = makeContext();
    m_activeCommand->begin(context);
    emit activeCommandChanged(m_activeCommand.get());
    finalizeIfFinished();
    m_canvas.update();
  }

  void CommandManager::cancelActiveCommand() {
    if (!m_activeCommand) {
      return;
    }

    CommandContext context = makeContext();
    m_activeCommand->cancel(context);
    clearActiveCommand();
  }

  void CommandManager::refreshActiveCommand() {
    dispatchActiveCommand([](Command &command, CommandContext &context) {
      command.onContextChanged(context);
    });
  }

  void CommandManager::handleMouseMove(const glm::dvec2 &worldPosition) {
    dispatchActiveCommand([&](Command &command, CommandContext &context) {
      command.onMouseMove(worldPosition, context);
    });
  }

  void CommandManager::handleMousePress(const glm::dvec2 &worldPosition, const Qt::MouseButton button) {
    dispatchActiveCommand([&](Command &command, CommandContext &context) {
      command.onMousePress(worldPosition, button, context);
    });
  }

  void CommandManager::handleKeyPress(QKeyEvent *event) {
    if (!m_activeCommand || !event) {
      return;
    }

    if (event->key() == Qt::Key_Escape) {
      cancelActiveCommand();
      event->accept();
      return;
    }

    dispatchActiveCommand([&](Command &command, CommandContext &context) {
      command.onKeyPress(*event, context);
    });
  }

  const Qadra::Core::TextEntityData *CommandManager::previewTextEntity() const noexcept {
    return m_activeCommand ? m_activeCommand->previewTextEntity() : nullptr;
  }

  CommandContext CommandManager::makeContext() {
    return {
      .document = m_document,
      .canvas = m_canvas,
      .requestRedraw = [this]() {
        m_canvas.update();
      },
      .appendPrompt = [this](const QString &prompt) {
        emit promptEmitted(prompt);
      },
      .notifyStateChanged = [this]() {
        finalizeIfFinished();
      }
    };
  }

  template <typename Invoker>
  void CommandManager::dispatchActiveCommand(Invoker &&invoker) {
    if (!m_activeCommand) {
      return;
    }

    CommandContext context = makeContext();
    std::forward<Invoker>(invoker)(*m_activeCommand, context);
    finalizeIfFinished();
  }

  void CommandManager::finalizeIfFinished() {
    if (!m_activeCommand || !m_activeCommand->isFinished()) {
      return;
    }

    clearActiveCommand();
  }

  void CommandManager::clearActiveCommand() {
    if (!m_activeCommand) {
      return;
    }

    emit activeCommandChanged(nullptr);
    m_activeCommand.reset();
    m_canvas.update();
  }
} // Qadra::Ui
