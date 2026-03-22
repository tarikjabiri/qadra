#include "GridPass.hpp"

#include <stdexcept>

#include "math/Grid.hpp"

namespace Qadra::Render {
  void GridPass::init(const QString &vertexSource, const QString &fragmentSource) {
    GL::Shader vertexShader(GL::Shader::Type::Vertex);
    GL::Shader fragmentShader(GL::Shader::Type::Fragment);

    if (!vertexShader.compile(vertexSource) || !fragmentShader.compile(fragmentSource)) {
      throw std::runtime_error("Vertex shader or fragment shaders are not compiled");
    }

    if (!m_program.link(vertexShader, fragmentShader)) {
      throw std::runtime_error("Program shader or fragment shaders are not linked");
    }

    m_vao.attribute({.index = 0, .size = 2, .type = GL_DOUBLE, .relativeOffset = 0});
    m_vao.attribute({.index = 1, .size = 4, .type = GL_DOUBLE, .relativeOffset = offsetof(Vertex, color)});
  }

  void GridPass::render(const Core::Camera &camera) const {
    std::vector<Vertex> vertices;
    std::vector<Math::Grid::Line> lines = Math::Grid::compute(camera);

    if (lines.size() == 0) return;

    constexpr auto minorColor = glm::dvec4(0.72, 0.76, 0.82, 0.05);
    constexpr auto majorColor = glm::dvec4(0.78, 0.82, 0.88, 0.11);

    for (const auto &[start, end, major]: lines) {
      vertices.push_back({start, major ? majorColor : minorColor});
      vertices.push_back({end, major ? majorColor : minorColor});
    }

    m_vbo.allocate(
      static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
      GL::Buffer::Usage::DynamicDraw,
      vertices.data()
    );

    m_vao.attachVertexBuffer(0, m_vbo, 0, sizeof(Vertex));

    m_vao.bind();
    m_program.bind();
    m_program.uniform("u_viewProjection", camera.viewProjection());

    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(lines.size() * 2));
  }
}
