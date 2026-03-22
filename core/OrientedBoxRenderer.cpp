#include "OrientedBoxRenderer.hpp"

#include <cstddef>
#include <span>
#include <stdexcept>

#include "Shader.hpp"

namespace Qadra::Core {
  void OrientedBoxRenderer::init(const QString &vertexSource, const QString &fragmentSource) {
    GL::Shader vertexShader(GL::Shader::Type::Vertex);
    GL::Shader fragmentShader(GL::Shader::Type::Fragment);

    if (!vertexShader.compile(vertexSource) || !fragmentShader.compile(fragmentSource)) {
      throw std::runtime_error("Oriented box shaders are not compiled");
    }

    if (!m_program.link(vertexShader, fragmentShader)) {
      throw std::runtime_error("Oriented box shader program is not linked");
    }

    m_vertexArray.attribute({.index = 0, .size = 2, .type = GL_FLOAT, .relativeOffset = offsetof(Vertex, position)});
    m_vertexArray.attribute({.index = 1, .size = 4, .type = GL_FLOAT, .relativeOffset = offsetof(Vertex, color)});
    m_vertexArray.attachVertexBuffer(0, m_vertexBuffer, 0, sizeof(Vertex));

    m_initialized = true;
  }

  void OrientedBoxRenderer::begin(const Camera &camera) {
    if (!m_initialized) {
      throw std::runtime_error("Oriented box renderer is not initialized");
    }

    m_vertices.clear();
    m_camera = &camera;
  }

  void OrientedBoxRenderer::draw(const std::array<glm::dvec2, 4> &corners, const glm::vec4 &color) {
    if (!m_camera) {
      return;
    }

    m_vertices.push_back({.position = glm::vec2(corners[0]), .color = color});
    m_vertices.push_back({.position = glm::vec2(corners[1]), .color = color});
    m_vertices.push_back({.position = glm::vec2(corners[2]), .color = color});
    m_vertices.push_back({.position = glm::vec2(corners[0]), .color = color});
    m_vertices.push_back({.position = glm::vec2(corners[2]), .color = color});
    m_vertices.push_back({.position = glm::vec2(corners[3]), .color = color});
  }

  void OrientedBoxRenderer::end() {
    if (!m_camera || m_vertices.empty()) {
      m_camera = nullptr;
      return;
    }

    m_vertexBuffer.allocate(std::span<const Vertex>(m_vertices), GL::Buffer::Usage::DynamicDraw);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_vertexArray.bind();
    m_program.bind();
    m_program.uniform("u_viewProjection", glm::mat4(m_camera->viewProjection()));

    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_vertices.size()));

    glDisable(GL_BLEND);
    m_camera = nullptr;
  }
} // Qadra::Core
