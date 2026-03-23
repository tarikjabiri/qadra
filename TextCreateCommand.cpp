#include "TextCreateCommand.hpp"

#include <cmath>
#include <utility>

#include "Canvas.hpp"
#include "Document.hpp"
#include "TextEntity.hpp"

namespace {
  constexpr double pi = 3.14159265358979323846;

  [[nodiscard]] double degreesToRadians(const double degrees) {
    return degrees * pi / 180.0;
  }

  [[nodiscard]] glm::vec4 toGlColor(const QColor &color) {
    return {
      static_cast<float>(color.redF()),
      static_cast<float>(color.greenF()),
      static_cast<float>(color.blueF()),
      static_cast<float>(color.alphaF())
    };
  }
}

namespace Qadra::Ui {
  QString TextCreateCommand::id() const {
    return "TEXT";
  }

  void TextCreateCommand::begin(CommandContext &context) {
    cacheContext(context);

    m_finished = false;
    m_insertionPoint.reset();
    m_previewTextEntity.reset();
    m_heightWorld = 32.0;
    m_rotationDegrees = 0.0;
    m_content.clear();
    m_color = QColor::fromRgbF(
      context.canvas.defaultTextColor().r,
      context.canvas.defaultTextColor().g,
      context.canvas.defaultTextColor().b,
      context.canvas.defaultTextColor().a
    );
    m_fontKey = QString::fromStdString(context.canvas.defaultFontKey());

    if (m_fontKey.isEmpty() || !context.canvas.font(m_fontKey.toStdString())) {
      m_appendPrompt("TEXT: no usable font is loaded, so text placement is unavailable.");
      m_finished = true;
      m_notifyStateChanged();
      return;
    }

    m_appendPrompt("TEXT: click an insertion point. Typing will happen directly in the canvas.");
    m_requestRedraw();
  }

  void TextCreateCommand::cancel(CommandContext &context) {
    cacheContext(context);
    cancelInlineEditing(true);
  }

  void TextCreateCommand::onContextChanged(CommandContext &context) {
    cacheContext(context);
    if (m_canvas && m_canvas->hasActiveInlineTextEdit() && m_insertionPoint) {
      m_canvas->updateInlineTextEdit({
        .worldPosition = *m_insertionPoint,
        .heightWorld = m_heightWorld,
        .rotationRadians = degreesToRadians(m_rotationDegrees),
        .text = m_content,
        .fontKey = m_fontKey,
        .color = currentColor()
      });
    }
    updatePreview();
  }

  void TextCreateCommand::onMousePress(const glm::dvec2 &worldPosition,
                                       const Qt::MouseButton button,
                                       CommandContext &context) {
    cacheContext(context);
    if (button != Qt::LeftButton || m_finished || !hasActiveContext()) {
      return;
    }

    if (m_canvas->hasActiveInlineTextEdit()) {
      return;
    }

    m_insertionPoint = worldPosition;
    startInlineEditing();
  }

  const Qadra::Core::TextEntityData *TextCreateCommand::previewTextEntity() const {
    return m_previewTextEntity ? &*m_previewTextEntity : nullptr;
  }

  void TextCreateCommand::cacheContext(CommandContext &context) {
    m_document = &context.document;
    m_canvas = &context.canvas;
    m_requestRedraw = std::move(context.requestRedraw);
    m_appendPrompt = std::move(context.appendPrompt);
    m_notifyStateChanged = std::move(context.notifyStateChanged);
  }

  bool TextCreateCommand::hasActiveContext() const {
    return m_document && m_canvas &&
           static_cast<bool>(m_requestRedraw) &&
           static_cast<bool>(m_appendPrompt) &&
           static_cast<bool>(m_notifyStateChanged);
  }

  void TextCreateCommand::startInlineEditing() {
    if (!hasActiveContext() || !m_insertionPoint) {
      return;
    }

    if (m_fontKey.isEmpty() || !m_canvas->font(m_fontKey.toStdString())) {
      m_fontKey = QString::fromStdString(m_canvas->defaultFontKey());
    }

    if (m_fontKey.isEmpty() || !m_canvas->font(m_fontKey.toStdString())) {
      m_appendPrompt("TEXT: no usable font is loaded, so text placement is unavailable.");
      m_finished = true;
      m_notifyStateChanged();
      return;
    }

    updatePreview();

    Canvas::InlineTextEditRequest request{
      .worldPosition = *m_insertionPoint,
      .heightWorld = m_heightWorld,
      .rotationRadians = degreesToRadians(m_rotationDegrees),
      .text = m_content,
      .fontKey = m_fontKey,
      .color = currentColor()
    };
    Canvas::InlineTextEditCallbacks callbacks{
      .onTextChanged = [this](const QString &text) {
        m_content = text;
        updatePreview();
      },
      .onAccepted = [this](const QString &text) {
        acceptInlineEditing(text);
      },
      .onCancelled = [this]() {
        cancelInlineEditing(true);
      }
    };
    m_canvas->beginInlineTextEdit(request, std::move(callbacks));
    m_appendPrompt("TEXT: type in the canvas, press Enter to commit, or Esc to cancel.");
  }

  void TextCreateCommand::updatePreview() {
    m_previewTextEntity.reset();

    if (!hasActiveContext() || !m_insertionPoint || m_content.trimmed().isEmpty()) {
      m_requestRedraw();
      return;
    }

    Qadra::Core::Font *font = m_canvas->font(m_fontKey.toStdString());
    if (!font) {
      m_requestRedraw();
      return;
    }

    const Qadra::Core::TextEntityBuildInput input{
      .position = *m_insertionPoint,
      .heightWorld = m_heightWorld,
      .rotationRadians = degreesToRadians(m_rotationDegrees),
      .content = m_content.toUtf8().toStdString(),
      .fontKey = m_fontKey.toStdString(),
      .color = currentColor()
    };
    m_previewTextEntity = Qadra::Core::buildTextEntityData(0, input, *font);
    m_requestRedraw();
  }

  void TextCreateCommand::acceptInlineEditing(const QString &text) {
    if (!hasActiveContext()) {
      return;
    }

    m_content = text;
    updatePreview();

    if (!m_previewTextEntity || m_content.trimmed().isEmpty()) {
      cancelInlineEditing(true);
      return;
    }

    static_cast<void>(m_document->createText(*m_previewTextEntity));
    m_canvas->endInlineTextEdit();
    m_appendPrompt(QString("TEXT: created \"%1\".").arg(m_content));
    m_insertionPoint.reset();
    m_previewTextEntity.reset();
    m_finished = true;
    m_requestRedraw();
    m_notifyStateChanged();
  }

  void TextCreateCommand::cancelInlineEditing(const bool emitPrompt) {
    if (!hasActiveContext() || m_finished) {
      return;
    }

    m_canvas->endInlineTextEdit();
    m_insertionPoint.reset();
    m_previewTextEntity.reset();
    m_content.clear();
    m_finished = true;
    if (emitPrompt) {
      m_appendPrompt("TEXT: cancelled.");
    }
    m_requestRedraw();
    m_notifyStateChanged();
  }

  glm::vec4 TextCreateCommand::currentColor() const {
    return toGlColor(m_color);
  }
} // Qadra::Ui
