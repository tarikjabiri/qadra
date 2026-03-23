#include "TextRenderer.hpp"

#include <cmath>
#include <cstddef>
#include <stdexcept>

#include "Shader.hpp"

namespace Qadra::Core {
  void TextRenderer::init(const QString &vertexSource, const QString &fragmentSource) {
    GL::Shader vertexShader(GL::Shader::Type::Vertex);
    GL::Shader fragmentShader(GL::Shader::Type::Fragment);

    if (!vertexShader.compile(vertexSource) || !fragmentShader.compile(fragmentSource)) {
      throw std::runtime_error("Text shaders are not compiled");
    }

    if (!m_program.link(vertexShader, fragmentShader)) {
      throw std::runtime_error("Text shader program is not linked");
    }

    m_vao.attribute({.index = 0, .size = 2, .type = GL_FLOAT, .relativeOffset = offsetof(Vertex, position)});
    m_vao.attribute({.index = 1, .size = 2, .type = GL_FLOAT, .relativeOffset = offsetof(Vertex, uv)});
    m_vao.attribute({.index = 2, .size = 4, .type = GL_FLOAT, .relativeOffset = offsetof(Vertex, color)});
    m_vao.attachVertexBuffer(0, m_vbo, 0, sizeof(Vertex));

    m_initialized = true;
  }

  void TextRenderer::begin(Font &font, const Camera &camera) {
    if (!m_initialized) {
      throw std::runtime_error("Text renderer is not initialized");
    }

    m_vertices.clear();
    m_font = &font;
    m_camera = &camera;
  }

  void TextRenderer::draw(const std::string &text, const glm::dvec2 &position, const double height,
                          const double rotation, const glm::vec4 &color) {
    if (!m_font || !m_camera || text.empty() || height <= 0.0) {
      return;
    }

    const TextLayout layout = measureTextLayout(*m_font, text, height);
    if (layout.glyphs.empty()) {
      return;
    }

    const bool shouldRotate = std::abs(rotation) > 1e-12;
    const double sine = shouldRotate ? std::sin(rotation) : 0.0;
    const double cosine = shouldRotate ? std::cos(rotation) : 1.0;

    const auto rotateAroundOrigin = [&](const glm::dvec2 &point) {
      if (!shouldRotate) {
        return point;
      }

      const glm::dvec2 local = point - position;
      return glm::dvec2(
               local.x * cosine - local.y * sine,
               local.x * sine + local.y * cosine
             ) + position;
    };

    for (const TextLayoutGlyph &layoutGlyph: layout.glyphs) {
      const glm::dvec2 glyphSize = layoutGlyph.planeBounds.maximum - layoutGlyph.planeBounds.minimum;
      if (glyphSize.x <= 0.0 || glyphSize.y <= 0.0) {
        continue;
      }

      const double left = position.x + layoutGlyph.planeBounds.minimum.x;
      const double right = position.x + layoutGlyph.planeBounds.maximum.x;
      const double bottom = position.y + layoutGlyph.planeBounds.minimum.y;
      const double top = position.y + layoutGlyph.planeBounds.maximum.y;

      const glm::dvec2 p0 = rotateAroundOrigin(glm::dvec2(left, bottom));
      const glm::dvec2 p1 = rotateAroundOrigin(glm::dvec2(right, bottom));
      const glm::dvec2 p2 = rotateAroundOrigin(glm::dvec2(right, top));
      const glm::dvec2 p3 = rotateAroundOrigin(glm::dvec2(left, top));

      const glm::vec2 uv0(layoutGlyph.uvMin.x, layoutGlyph.uvMin.y);
      const glm::vec2 uv1(layoutGlyph.uvMax.x, layoutGlyph.uvMin.y);
      const glm::vec2 uv2(layoutGlyph.uvMax.x, layoutGlyph.uvMax.y);
      const glm::vec2 uv3(layoutGlyph.uvMin.x, layoutGlyph.uvMax.y);

      m_vertices.push_back({.position = glm::vec2(p0), .uv = uv0, .color = color});
      m_vertices.push_back({.position = glm::vec2(p1), .uv = uv1, .color = color});
      m_vertices.push_back({.position = glm::vec2(p2), .uv = uv2, .color = color});
      m_vertices.push_back({.position = glm::vec2(p0), .uv = uv0, .color = color});
      m_vertices.push_back({.position = glm::vec2(p2), .uv = uv2, .color = color});
      m_vertices.push_back({.position = glm::vec2(p3), .uv = uv3, .color = color});
    }
  }

  void TextRenderer::end() {
    if (!m_font || !m_camera || m_vertices.empty()) {
      m_font = nullptr;
      m_camera = nullptr;
      return;
    }

    m_vbo.allocate(std::span<const Vertex>(m_vertices), GL::Buffer::Usage::DynamicDraw);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_font->atlas().bind(0);
    m_vao.bind();
    m_program.bind();
    m_program.uniform("u_viewProjection", glm::mat4(m_camera->viewProjection()));
    m_program.uniform("u_texture", 0);
    m_program.uniform("u_distanceFieldRangePixels", static_cast<float>(m_font->distanceFieldRangePixels()));

    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_vertices.size()));

    glDisable(GL_BLEND);

    m_font = nullptr;
    m_camera = nullptr;
  }
} // Qadra::Core
