#ifndef QADRA_CORE_ORIENTEDBOXRENDERER_HPP
#define QADRA_CORE_ORIENTEDBOXRENDERER_HPP

#include <array>
#include <vector>

#include <QString>
#include <glm/glm.hpp>

#include "Buffer.hpp"
#include "Camera.hpp"
#include "Program.hpp"
#include "VertexArray.hpp"

namespace Qadra::Core {
  class OrientedBoxRenderer {
  public:
    struct Vertex {
      glm::vec2 position{0.0f};
      glm::vec4 color{1.0f};
    };

    void init(const QString &vertexSource, const QString &fragmentSource);

    void begin(const Camera &camera);

    void draw(const std::array<glm::dvec2, 4> &corners, const glm::vec4 &color);

    void end();

  private:
    GL::Buffer m_vertexBuffer;
    GL::VertexArray m_vertexArray;
    GL::Program m_program;
    std::vector<Vertex> m_vertices;
    const Camera *m_camera{};
    bool m_initialized{false};
  };
} // Qadra::Core

#endif // QADRA_CORE_ORIENTEDBOXRENDERER_HPP
