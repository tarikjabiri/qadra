#ifndef QADRA_CORE_LINERENDERER_HPP
#define QADRA_CORE_LINERENDERER_HPP

#include <vector>

#include <QString>
#include <glm/glm.hpp>

#include "Buffer.hpp"
#include "Camera.hpp"
#include "Program.hpp"
#include "VertexArray.hpp"

namespace Qadra::Core {
  class LineRenderer {
  public:
    struct Vertex {
      glm::vec2 position{0.0f};
      glm::vec4 color{1.0f};
    };

    void init(const QString &vertexSource, const QString &fragmentSource);

    void begin(const Camera &camera, const glm::vec2 &viewportSizePixels);

    void draw(const glm::dvec2 &from, const glm::dvec2 &to, const glm::vec4 &color);

    void end();

  private:
    GL::Buffer m_vertexBuffer;
    GL::VertexArray m_vertexArray;
    GL::Program m_program;
    std::vector<Vertex> m_vertices;
    const Camera *m_camera{};
    glm::vec2 m_viewportSizePixels{0.0f};
    bool m_initialized{false};
  };
} // Qadra::Core

#endif // QADRA_CORE_LINERENDERER_HPP
