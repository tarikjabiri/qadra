#include "LineRenderer.hpp"

#include <cstddef>
#include <span>
#include <stdexcept>

#include "Shader.hpp"

namespace Qadra::Core {
  void LineRenderer::init(const QString &vertexSource, const QString &fragmentSource) {
    GL::Shader vertexShader(GL::Shader::Type::Vertex);
    GL::Shader fragmentShader(GL::Shader::Type::Fragment);

    if (!vertexShader.compile(vertexSource) || !fragmentShader.compile(fragmentSource)) {
      throw std::runtime_error("Line shaders are not compiled");
    }

    if (!m_program.link(vertexShader, fragmentShader)) {
      throw std::runtime_error("Line shader program is not linked");
    }

    m_vertexArray.attribute({.index = 0, .size = 2, .type = GL_FLOAT, .relativeOffset = offsetof(Vertex, position)});
    m_vertexArray.attribute({.index = 1, .size = 4, .type = GL_FLOAT, .relativeOffset = offsetof(Vertex, color)});
    m_vertexArray.attachVertexBuffer(0, m_vertexBuffer, 0, sizeof(Vertex));

    m_initialized = true;
  }

  void LineRenderer::begin(const Camera &camera, const glm::vec2 &viewportSizePixels) {
    if (!m_initialized) {
      throw std::runtime_error("Line renderer is not initialized");
    }

    m_vertices.clear();
    m_camera = &camera;
    m_viewportSizePixels = viewportSizePixels;
  }

  void LineRenderer::draw(const glm::dvec2 &from, const glm::dvec2 &to, const glm::vec4 &color) {
    if (!m_camera) {
      return;
    }

    m_vertices.push_back({.position = glm::vec2(from), .color = color});
    m_vertices.push_back({.position = glm::vec2(to), .color = color});
  }

  void LineRenderer::end() {
    if (!m_camera || m_vertices.empty()) {
      m_camera = nullptr;
      return;
    }

    m_vertexBuffer.allocate(std::span<const Vertex>(m_vertices), GL::Buffer::Usage::DynamicDraw);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(1.0f);

    m_vertexArray.bind();
    m_program.bind();
    m_program.uniform("u_viewProjection", glm::mat4(m_camera->viewProjection()));
    m_program.uniform("u_viewportSizePixels", m_viewportSizePixels);

    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_vertices.size()));

    glDisable(GL_BLEND);
    m_camera = nullptr;
  }
} // Qadra::Core
