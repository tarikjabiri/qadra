#ifndef QADRA_RENDER_GRIDPASS_HPP
#define QADRA_RENDER_GRIDPASS_HPP

#include "Buffer.hpp"
#include "Camera.hpp"
#include "Program.hpp"
#include "VertexArray.hpp"

namespace Qadra::Render {
  class GridPass {
  public:
    struct Vertex {
      glm::dvec2 position;
      glm::dvec4 color;
    };

    GridPass() = default;

    void init(const QString &vertexSource, const QString &fragmentSource);

    void render(const Core::Camera &camera) const;

  private:
    GL::Buffer m_vbo;
    GL::VertexArray m_vao;
    GL::Program m_program;
  };
}

#endif //QADRA_RENDER_GRIDPASS_HPP
