#ifndef QADRA_CORE_TEXTRENDERER_HPP
#define QADRA_CORE_TEXTRENDERER_HPP

#include <QString>
#include <glm/glm.hpp>

#include <string>
#include <vector>

#include "Buffer.hpp"
#include "Camera.hpp"
#include "Font.hpp"
#include "Program.hpp"
#include "VertexArray.hpp"

namespace Qadra::Core {
  class TextRenderer {
  public:
    struct Vertex {
      glm::vec2 position{0.0f};
      glm::vec2 uv{0.0f};
      glm::vec4 color{1.0f};
    };

    void init(const QString &vertexSource, const QString &fragmentSource);

    void begin(Font &font, const Camera &camera);

    void draw(const std::string &text, const glm::dvec2 &position, double height, double rotation,
              const glm::vec4 &color);

    void end();

  private:
    GL::Buffer m_vbo;
    GL::VertexArray m_vao;
    GL::Program m_program;
    std::vector<Vertex> m_vertices;
    Font *m_font{};
    const Camera *m_camera{};
    bool m_initialized{false};
  };
} // Qadra::Core

#endif // QADRA_CORE_TEXTRENDERER_HPP
