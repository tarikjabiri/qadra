#include <glad/gl.h>

#include "Canvas.hpp"

#include <algorithm>
#include <cmath>
#include <memory>
#include <span>
#include <stdexcept>

#include <QCoreApplication>
#include <QCursor>
#include <QFile>
#include <QGuiApplication>
#include <QInputMethod>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QTextStream>
#include <QWheelEvent>

#include "CommandManager.hpp"
#include "Document.hpp"
#include "TextEntity.hpp"

namespace {
  int viewportPixels(const int logicalPixels, const qreal devicePixelRatio) {
    return static_cast<int>(std::lround(static_cast<double>(logicalPixels) * devicePixelRatio));
  }

  glm::dvec2 viewportPixels(const QPointF &logicalPosition, const qreal devicePixelRatio) {
    return {
      logicalPosition.x() * devicePixelRatio,
      logicalPosition.y() * devicePixelRatio
    };
  }

  Qadra::Core::TextLayoutBounds expandBounds(const Qadra::Core::TextLayoutBounds &bounds, const double padding) {
    return {
      .minimum = bounds.minimum - glm::dvec2(padding, padding),
      .maximum = bounds.maximum + glm::dvec2(padding, padding)
    };
  }

  [[nodiscard]] bool intersects(const Qadra::Core::TextLayoutBounds &left,
                                const Qadra::Core::TextLayoutBounds &right) {
    return left.minimum.x < right.maximum.x &&
           left.maximum.x > right.minimum.x &&
           left.minimum.y < right.maximum.y &&
           left.maximum.y > right.minimum.y;
  }

  constexpr double minimumReadableTextHeightPixels = 10.0;

}

namespace Qadra::Ui {
  Canvas::Canvas(QWidget *parent)
    : QOpenGLWidget(parent),
      m_inlineTextEditController(std::make_unique<InlineTextEditSessionController>(*this, this)) {
    QSurfaceFormat format;
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(4, 6);
    format.setAlphaBufferSize(8);
    format.setSwapInterval(0);
    format.setSamples(4);
    setFormat(format);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setAttribute(Qt::WA_InputMethodEnabled, true);

    QObject::connect(m_inlineTextEditController.get(), &InlineTextEditSessionController::stateChanged, this, [this]() {
      updateInputMethodState();
      update();
    });
  }

  QString Canvas::loadShaderSource(const QString &filename) {
    const QString path = QCoreApplication::applicationDirPath() + "/shaders/" + filename;
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      throw std::runtime_error("Failed to open shader: " + path.toStdString());
    }

    return QTextStream(&file).readAll();
  }

  void Canvas::setDocument(Qadra::Core::Document *document) {
    m_document = document;
    update();
  }

  void Canvas::setCommandManager(CommandManager *commandManager) {
    m_commandManager = commandManager;
    update();
  }

  void Canvas::setDebugTextOverlayEnabled(const bool enabled) {
    if (m_debugTextOverlayEnabled == enabled) {
      return;
    }

    m_debugTextOverlayEnabled = enabled;
    rebuildDebugTextOverlay();
    update();
  }

  std::string Canvas::defaultFontKey() const {
    return m_fontCatalog.defaultFontKey();
  }

  glm::vec4 Canvas::defaultTextColor() const noexcept {
    return {0.89f, 0.93f, 1.00f, 0.95f};
  }

  Qadra::Core::Font *Canvas::font(const std::string_view fontKey) {
    return m_fontCatalog.font(fontKey);
  }

  const Qadra::Core::Font *Canvas::font(const std::string_view fontKey) const {
    return m_fontCatalog.font(fontKey);
  }

  void Canvas::beginInlineTextEdit(const InlineTextEditRequest &request, InlineTextEditCallbacks callbacks) {
    endInlineTextEdit();

    InlineTextEditRequest sessionRequest = request;
    if (Qadra::Core::Font *editingFont = font(request.fontKey.toStdString())) {
      sessionRequest.stableLineBoundsLocal = Qadra::Core::defaultInlineEditingLineBounds(*editingFont, request.heightWorld);
    }
    m_inlineTextEditController->begin(sessionRequest, std::move(callbacks));
    setCursor(Qt::IBeamCursor);
    updateInputMethodState();
    update();
  }

  void Canvas::updateInlineTextEdit(const InlineTextEditRequest &request) {
    if (!m_inlineTextEditController || !m_inlineTextEditController->hasActiveSession()) {
      return;
    }

    InlineTextEditRequest sessionRequest = request;
    if (Qadra::Core::Font *editingFont = font(request.fontKey.toStdString())) {
      sessionRequest.stableLineBoundsLocal = Qadra::Core::defaultInlineEditingLineBounds(*editingFont, request.heightWorld);
    }
    m_inlineTextEditController->update(sessionRequest);
    updateInputMethodState();
    update();
  }

  void Canvas::endInlineTextEdit() {
    if (!m_inlineTextEditController || !m_inlineTextEditController->hasActiveSession()) {
      return;
    }

    m_inlineTextEditController->end();
    if (QInputMethod *inputMethod = QGuiApplication::inputMethod()) {
      inputMethod->reset();
    }
    unsetCursor();
    updateInputMethodState();
    setFocus(Qt::OtherFocusReason);
    update();
  }

  bool Canvas::hasActiveInlineTextEdit() const noexcept {
    return m_inlineTextEditController && m_inlineTextEditController->hasActiveSession();
  }

  void Canvas::initializeGL() {
    if (gladLoadGL(&Canvas::getProcAddress) == 0) {
      QMessageBox::critical(this, tr("OpenGL"), tr("Failed to load OpenGL functions."));
      return;
    }

    m_gridPass.emplace();
    m_lineRenderer.emplace();
    m_orientedBoxRenderer.emplace();
    m_textRenderer.emplace();

    const QString gridVertexSource = loadShaderSource("grid.vertex.glsl");
    const QString gridFragmentSource = loadShaderSource("grid.fragment.glsl");
    const QString lineVertexSource = loadShaderSource("line.vertex.glsl");
    const QString lineFragmentSource = loadShaderSource("line.fragment.glsl");
    const QString orientedBoxVertexSource = loadShaderSource("oriented-box.vertex.glsl");
    const QString orientedBoxFragmentSource = loadShaderSource("oriented-box.fragment.glsl");
    const QString textVertexSource = loadShaderSource("text.vertex.glsl");
    const QString textFragmentSource = loadShaderSource("text.fragment.glsl");

    m_gridPass->init(gridVertexSource, gridFragmentSource);
    m_lineRenderer->init(lineVertexSource, lineFragmentSource);
    m_orientedBoxRenderer->init(orientedBoxVertexSource, orientedBoxFragmentSource);
    m_textRenderer->init(textVertexSource, textFragmentSource);

    m_fontCatalog.loadSystemFonts();
    emit fontsChanged();
    rebuildDebugTextOverlay();
    m_initialized = true;
  }

  void Canvas::paintGL() {
    if (!m_initialized) {
      return;
    }

    const qreal devicePixelRatioValue = devicePixelRatioF();
    const GLsizei viewportWidthPixels = viewportPixels(width(), devicePixelRatioValue);
    const GLsizei viewportHeightPixels = viewportPixels(height(), devicePixelRatioValue);

    if (viewportWidthPixels <= 0 || viewportHeightPixels <= 0) {
      return;
    }

    if (m_camera.width() != viewportWidthPixels || m_camera.height() != viewportHeightPixels) {
      if (m_hasInitializedCameraViewport) {
        m_camera.resizePreserveViewportOrigin(viewportWidthPixels, viewportHeightPixels);
      } else {
        m_camera.resize(viewportWidthPixels, viewportHeightPixels);
        m_hasInitializedCameraViewport = true;
      }
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);

    glViewport(0, 0, viewportWidthPixels, viewportHeightPixels);
    glClearColor(0.09f, 0.10f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_gridPass->render(
      m_camera,
      glm::vec2(static_cast<float>(viewportWidthPixels), static_cast<float>(viewportHeightPixels))
    );

    std::vector<const Qadra::Core::TextEntityData *> visibleEntities;
    if (m_document) {
      visibleEntities.reserve(m_document->entities().size() + m_debugTextOverlay.textEntities().size() + 1);
      for (const Qadra::Core::TextEntityData &textEntity: m_document->entities()) {
        visibleEntities.push_back(&textEntity);
      }
    }

    if (m_debugTextOverlayEnabled) {
      for (const Qadra::Core::TextEntityData &textEntity: m_debugTextOverlay.textEntities()) {
        visibleEntities.push_back(&textEntity);
      }
    }

    if (m_commandManager && !hasActiveInlineTextEdit()) {
      if (const Qadra::Core::TextEntityData *previewTextEntity = m_commandManager->previewTextEntity()) {
        visibleEntities.push_back(previewTextEntity);
      }
    }

    if (!visibleEntities.empty()) {
      const glm::dvec2 topLeftWorld = m_camera.screenToWorld(glm::dvec2(0.0, 0.0));
      const glm::dvec2 bottomRightWorld = m_camera.screenToWorld(glm::dvec2(viewportWidthPixels, viewportHeightPixels));
      const Qadra::Core::TextLayoutBounds visibleWorldBounds = expandBounds({
        .minimum = glm::dvec2(topLeftWorld.x, bottomRightWorld.y),
        .maximum = glm::dvec2(bottomRightWorld.x, topLeftWorld.y)
      }, 220.0 / std::max(m_camera.zoom(), 1e-6));

      renderTextEntities(
        std::span<const Qadra::Core::TextEntityData *const>(visibleEntities.data(), visibleEntities.size()),
        visibleWorldBounds
      );
    }
    renderInlineTextEdit();
  }

  void Canvas::resizeGL(const int, const int) {
    const qreal devicePixelRatioValue = devicePixelRatioF();
    const int viewportWidthPixels = viewportPixels(width(), devicePixelRatioValue);
    const int viewportHeightPixels = viewportPixels(height(), devicePixelRatioValue);

    if (m_hasInitializedCameraViewport) {
      m_camera.resizePreserveViewportOrigin(viewportWidthPixels, viewportHeightPixels);
    } else {
      m_camera.resize(viewportWidthPixels, viewportHeightPixels);
      m_hasInitializedCameraViewport = true;
    }

    updateInputMethodState();
  }

  void Canvas::rebuildDebugTextOverlay() {
    if (!m_debugTextOverlayEnabled || m_fontCatalog.empty()) {
      m_debugTextOverlay.clear();
      return;
    }

    std::vector<DebugTextOverlay::FontEntry> fontEntries;
    fontEntries.reserve(m_fontCatalog.loadedFonts().size());
    for (const CanvasFontCatalog::LoadedFont &loadedFont: m_fontCatalog.loadedFonts()) {
      fontEntries.push_back({
        .key = loadedFont.key,
        .font = loadedFont.font.get()
      });
    }

    m_debugTextOverlay.rebuild(std::span<const DebugTextOverlay::FontEntry>(fontEntries.data(), fontEntries.size()));
  }

  void Canvas::renderTextEntities(std::span<const Qadra::Core::TextEntityData *const> textEntities,
                                  const Qadra::Core::TextLayoutBounds &visibleWorldBounds) {
    if (!m_textRenderer || !m_orientedBoxRenderer || m_fontCatalog.empty() || textEntities.empty()) {
      return;
    }

    m_orientedBoxRenderer->begin(m_camera);

    for (const CanvasFontCatalog::LoadedFont &loadedFont: m_fontCatalog.loadedFonts()) {
      m_textRenderer->begin(*loadedFont.font, m_camera);

      for (const Qadra::Core::TextEntityData *textEntity: textEntities) {
        if (!textEntity || textEntity->fontKey != loadedFont.key) {
          continue;
        }

        if (!intersects(textEntity->worldBounds, visibleWorldBounds)) {
          continue;
        }

        const double pixelHeight = textEntity->heightWorld * m_camera.zoom();
        if (pixelHeight < minimumReadableTextHeightPixels) {
          const std::array<glm::dvec2, 4> orientedCorners = Qadra::Core::orientedTextCorners(
            textEntity->position,
            textEntity->localBounds,
            textEntity->rotationRadians
          );
          m_orientedBoxRenderer->draw(
            orientedCorners,
            glm::vec4(textEntity->color.r, textEntity->color.g, textEntity->color.b, textEntity->color.a * 0.55f)
          );
          continue;
        }

        m_textRenderer->draw(
          textEntity->content,
          textEntity->position,
          textEntity->heightWorld,
          textEntity->rotationRadians,
          textEntity->color
        );
      }

      m_textRenderer->end();
    }

    m_orientedBoxRenderer->end();
  }

  void Canvas::renderInlineTextEdit() {
    if (!m_inlineTextEditController || !m_textRenderer || !m_orientedBoxRenderer || !m_lineRenderer) {
      return;
    }

    const std::optional<InlineTextEditSessionController::Snapshot> snapshot = m_inlineTextEditController->snapshot();
    if (!snapshot) {
      return;
    }

    Qadra::Core::Font *editingFont = font(snapshot->fontKey.toStdString());
    if (!editingFont) {
      return;
    }

    const qreal devicePixelRatioValue = devicePixelRatioF();
    const glm::vec2 viewportSizePixels(
      static_cast<float>(viewportPixels(width(), devicePixelRatioValue)),
      static_cast<float>(viewportPixels(height(), devicePixelRatioValue))
    );
    m_inlineTextEditPresenter.render(
      *snapshot,
      *editingFont,
      {
        .camera = m_camera,
        .textRenderer = *m_textRenderer,
        .orientedBoxRenderer = *m_orientedBoxRenderer,
        .lineRenderer = *m_lineRenderer,
        .viewportSizePixels = viewportSizePixels
      }
    );
  }

  void Canvas::updateInputMethodState() {
    if (m_inlineTextEditController && m_inlineTextEditController->hasActiveSession()) {
      m_inlineTextEditController->setInputCursorRectangleLogical(inlineCaretRectangleLogical());
    }
    if (QInputMethod *inputMethod = QGuiApplication::inputMethod()) {
      inputMethod->update(Qt::ImCursorRectangle |
                          Qt::ImCursorPosition |
                          Qt::ImAnchorPosition |
                          Qt::ImSurroundingText |
                          Qt::ImCurrentSelection);
    }
  }

  std::optional<int> Canvas::inlineCursorIndexAtWorldPosition(const glm::dvec2 &worldPosition) {
    if (!m_inlineTextEditController || !m_inlineTextEditController->hasActiveSession()) {
      return std::nullopt;
    }

    const std::optional<InlineTextEditSessionController::Snapshot> snapshot = m_inlineTextEditController->snapshot();
    if (!snapshot) {
      return std::nullopt;
    }

    Qadra::Core::Font *editingFont = font(snapshot->fontKey.toStdString());
    if (!editingFont) {
      return std::nullopt;
    }

    if (m_inlineTextEditController->hasPreeditText()) {
      return m_inlineTextEditController->cursorPosition();
    }

    return m_inlineTextEditPresenter.cursorIndexAtWorldPosition(*snapshot, worldPosition, *editingFont);
  }

  QRectF Canvas::inlineCaretRectangleLogical() {
    if (!m_inlineTextEditController || !m_inlineTextEditController->hasActiveSession()) {
      return {};
    }

    const std::optional<InlineTextEditSessionController::Snapshot> snapshot = m_inlineTextEditController->snapshot();
    if (!snapshot) {
      return {};
    }

    Qadra::Core::Font *editingFont = font(snapshot->fontKey.toStdString());
    if (!editingFont) {
      return {};
    }

    return m_inlineTextEditPresenter.caretRectangleLogical(*snapshot, *editingFont, m_camera, devicePixelRatioF());
  }

  QFunctionPointer Canvas::getProcAddress(const char *procName) {
    if (QOpenGLContext *context = QOpenGLContext::currentContext()) {
      return context->getProcAddress(procName);
    }
    return nullptr;
  }

  void Canvas::mousePressEvent(QMouseEvent *event) {
    if (m_inlineTextEditController && m_inlineTextEditController->hasActiveSession() && event->button() == Qt::LeftButton) {
      const glm::dvec2 worldPosition = m_camera.screenToWorld(viewportPixels(event->position(), devicePixelRatioF()));
      if (const std::optional<int> cursorIndex = inlineCursorIndexAtWorldPosition(worldPosition)) {
        m_inlineTextEditController->setSelectionAnchor(m_inlineTextEditController->cursorPosition());
        m_inlineTextEditController->focus(Qt::MouseFocusReason);
        if (event->modifiers().testFlag(Qt::ShiftModifier)) {
          const int selectionStart = std::min(m_inlineTextEditController->selectionAnchor(), *cursorIndex);
          const int selectionLength = std::abs(*cursorIndex - m_inlineTextEditController->selectionAnchor());
          m_inlineTextEditController->setSelection(selectionStart, selectionLength);
        } else {
          m_inlineTextEditController->setCursorPosition(*cursorIndex);
          m_inlineTextEditController->setSelectionAnchor(*cursorIndex);
        }
        m_inlineTextEditController->setSelecting(true);
        event->accept();
        return;
      }
    }

    if (event->button() == Qt::MiddleButton) {
      if (!hasActiveInlineTextEdit()) {
        setFocus(Qt::MouseFocusReason);
      }
      m_panning = true;
      const glm::dvec2 positionPixels = viewportPixels(event->position(), devicePixelRatioF());
      m_lastMousePosition = QPointF(positionPixels.x, positionPixels.y);
      setCursor(QCursor(Qt::ClosedHandCursor));
      event->accept();
      return;
    }

    if (m_commandManager && m_commandManager->activeCommand()) {
      if (!hasActiveInlineTextEdit()) {
        setFocus(Qt::MouseFocusReason);
      }
      const glm::dvec2 positionPixels = viewportPixels(event->position(), devicePixelRatioF());
      m_commandManager->handleMousePress(m_camera.screenToWorld(positionPixels), event->button());
      event->accept();
      return;
    }

    QOpenGLWidget::mousePressEvent(event);
  }

  void Canvas::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && m_inlineTextEditController && m_inlineTextEditController->selecting()) {
      m_inlineTextEditController->setSelecting(false);
      event->accept();
      return;
    }

    if (event->button() == Qt::MiddleButton) {
      m_panning = false;
      if (hasActiveInlineTextEdit()) {
        setCursor(QCursor(Qt::IBeamCursor));
      } else {
        setCursor(QCursor(Qt::ArrowCursor));
      }
      event->accept();
      return;
    }

    QOpenGLWidget::mouseReleaseEvent(event);
  }

  void Canvas::mouseMoveEvent(QMouseEvent *event) {
    const glm::dvec2 positionPixels = viewportPixels(event->position(), devicePixelRatioF());

    if (m_inlineTextEditController && m_inlineTextEditController->selecting() && (event->buttons() & Qt::LeftButton)) {
      if (const std::optional<int> cursorIndex = inlineCursorIndexAtWorldPosition(m_camera.screenToWorld(positionPixels))) {
        const int selectionStart = std::min(m_inlineTextEditController->selectionAnchor(), *cursorIndex);
        const int selectionLength = std::abs(*cursorIndex - m_inlineTextEditController->selectionAnchor());
        m_inlineTextEditController->setSelection(selectionStart, selectionLength);
      }
      event->accept();
      return;
    }

    if (m_panning) {
      const QPointF currentMousePosition(positionPixels.x, positionPixels.y);
      const QPointF delta = currentMousePosition - m_lastMousePosition;
      m_camera.pan(glm::dvec2(-delta.x(), delta.y()));
      m_lastMousePosition = currentMousePosition;
      updateInputMethodState();
      update();
      event->accept();
      return;
    }

    if (m_commandManager && m_commandManager->activeCommand()) {
      m_commandManager->handleMouseMove(m_camera.screenToWorld(positionPixels));
      event->accept();
      return;
    }

    QOpenGLWidget::mouseMoveEvent(event);
  }

  void Canvas::keyPressEvent(QKeyEvent *event) {
    if (m_commandManager && m_commandManager->activeCommand()) {
      m_commandManager->handleKeyPress(event);
      if (event->isAccepted()) {
        return;
      }
    }

    QOpenGLWidget::keyPressEvent(event);
  }

  void Canvas::wheelEvent(QWheelEvent *event) {
    const float delta = event->angleDelta().y() / 105.0f;
    const glm::dvec2 mouseScreenPixels = viewportPixels(event->position(), devicePixelRatioF());
    const glm::dvec2 mouseWorld = m_camera.screenToWorld(mouseScreenPixels);
    m_camera.zoom(std::pow(1.1f, delta), mouseWorld);

    updateInputMethodState();
    update();
    event->accept();
  }
} // Qadra::Ui
