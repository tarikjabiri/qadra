#include <glad/gl.h>
#include "Canvas.hpp"

#include <cmath>
#include <QOpenGLContext>
#include <QMessageBox>
#include <QMainWindow>
#include <QWheelEvent>
#include <QFile>

#include "Program.hpp"
#include "Shader.hpp"


namespace Qadra::Ui {
  Canvas::Canvas() {
    setSurfaceType(OpenGLSurface);

    QSurfaceFormat format;
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(4, 6);
    format.setAlphaBufferSize(8);
    format.setSwapInterval(0);
    format.setSamples(4);
    setFormat(format);

    m_context = new QOpenGLContext(this);
    m_context->setFormat(requestedFormat());
  }

  QString Canvas::loadShaderSource(const QString &filename) {
    const QString path = QCoreApplication::applicationDirPath() + "/shaders/" + filename;
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      throw std::runtime_error("Failed to open shader: " + path.toStdString());
    }

    return QTextStream(&file).readAll();
  }

  void Canvas::exposeEvent(QExposeEvent *expose_event) {
    if (isExposed()) {
      if (!m_initialized) {
        initialize();
        m_initialized = true;

        m_buffer.emplace();
        m_vertexArray.emplace();
        m_program.emplace();

        m_programGrid.emplace();
        m_grid.emplace();

        m_gridPass.emplace();

        const auto gridVertexSource = loadShaderSource("grid.vertex.glsl");
        const auto gridFragmentSource = loadShaderSource("grid.fragment.glsl");

        m_gridPass->init(gridVertexSource, gridFragmentSource);

        GL::Shader gridVertexShader(GL::Shader::Type::Vertex);
        GL::Shader gridFragmentShader(GL::Shader::Type::Fragment);

        if (!gridVertexShader.compile(gridVertexSource) || !gridFragmentShader.compile(gridFragmentSource)) {
          throw std::runtime_error("Vertex shader or fragment shaders are not compiled");
        }

        if (!m_programGrid->link(gridVertexShader, gridFragmentShader)) {
          throw std::runtime_error("Gird program shader or fragment shaders are not linked");
        }

        constexpr float vertices[] = {
          -0.5f * 1000, -0.5f * 1000, 0.0f * 1000,
          0.5f * 1000, -0.5f * 1000, 0.0f * 1000,
          0.0f * 1000, 0.5f * 1000, 0.0f * 1000
        };

        m_vertexArray->bind();

        m_buffer->bind();
        m_buffer->allocate(std::span(vertices));

        const auto vertexShaderSource = loadShaderSource("vertex.glsl");
        const auto fragmentShaderSource = loadShaderSource("fragment.glsl");

        GL::Shader vertexShader(GL::Shader::Type::Vertex);
        GL::Shader fragmentShader(GL::Shader::Type::Fragment);

        if (!vertexShader.compile(vertexShaderSource) || !fragmentShader.compile(fragmentShaderSource)) {
          throw std::runtime_error("Vertex shader or fragment shaders are not compiled");
        }


        if (!m_program->link(vertexShader, fragmentShader)) {
          throw std::runtime_error("Program shader or fragment shaders are not linked");
        }

        m_program->bind();

        constexpr GL::VertexArray::Attribute position{
          .index = 0,
          .size = 3,
          .type = GL_FLOAT,
          .relativeOffset = 0,
        };
        m_vertexArray->attribute(position);

        m_vertexArray->attachVertexBuffer(0, m_buffer.value(), 0, 3 * sizeof(float));
      }
      render();
    }
  }

  bool Canvas::event(QEvent *event) {
    if (event->type() == QEvent::UpdateRequest) {
      render();
      return true;
    }
    return QWindow::event(event);
  }

  void Canvas::initialize() {
    if (!m_context->create()) {
      delete m_context;
      QMessageBox::critical(nullptr, tr("Context is not created"), tr("Context is not created"));
    }

    if (m_context->makeCurrent(this)) {
      if (gladLoadGL(&Canvas::getProcAddress) == 0) {
        QMessageBox::critical(nullptr, tr("QMessage"), tr("QMessage"));
      }
    }
  }

  void Canvas::render() {
    m_context->makeCurrent(this);

    const GLsizei viewportWidthPixels = std::floor(width() * devicePixelRatio());
    const GLsizei viewportHeightPixels = std::floor(height() * devicePixelRatio());

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);

    glViewport(0, 0, viewportWidthPixels, viewportHeightPixels);
    glClearColor(0.09f, 0.10f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);


    m_gridPass->render(m_camera, glm::vec2(static_cast<float>(viewportWidthPixels),
                                           static_cast<float>(viewportHeightPixels)));

    m_vertexArray->bind();
    m_buffer->bind();
    m_program->bind();
    m_program->uniform("u_viewProjection", m_camera.viewProjection());
    glDrawArrays(GL_TRIANGLES, 0, 3);

    m_context->swapBuffers(this);
  }

  QFunctionPointer Canvas::getProcAddress(const char *procName) {
    return QOpenGLContext::currentContext()->getProcAddress(procName);
  }

  void Canvas::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton) {
      m_panning = true;
      m_lastMousePosition = event->position();
      setCursor(QCursor(Qt::ClosedHandCursor));
      event->accept();
    } else {
      QWindow::mousePressEvent(event);
    }
  }

  void Canvas::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton) {
      m_panning = false;
      setCursor(QCursor(Qt::ArrowCursor));
      event->accept();
    } else {
      QWindow::mouseReleaseEvent(event);
    }
  }

  void Canvas::mouseMoveEvent(QMouseEvent *event) {
    if (m_panning) {
      const QPointF delta = event->position() - m_lastMousePosition;
      m_camera.pan(glm::vec2(-delta.x(), delta.y()));
      m_lastMousePosition = event->position();
      requestUpdate();
      event->accept();
    } else {
      QWindow::mouseMoveEvent(event);
    }
  }

  void Canvas::wheelEvent(QWheelEvent *event) {
    const float delta = event->angleDelta().y() / 120.0f;
    const glm::vec2 mouseScreen(event->position().x(), event->position().y());
    const glm::vec2 mouseWorld = m_camera.screenToWorld(mouseScreen);
    m_camera.zoom(std::pow(1.1f, delta), mouseWorld);

    requestUpdate();
    event->accept();
  }

  void Canvas::resizeEvent(QResizeEvent *event) {
    m_camera.resize(width(), height());
    QWindow::resizeEvent(event);
  }
} // Ui
// Qadra
