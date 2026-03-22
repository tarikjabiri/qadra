#include "Grid.hpp"

namespace Qadra::Core {
  Grid::Grid() {
  }

  void Grid::update(const Camera &camera) {
    buildLines(camera);

    if (!m_buffer) {
      m_buffer.emplace();
      m_vao.emplace();

      m_vao->attribute({.index = 0, .size = 2, .type = GL_DOUBLE, .relativeOffset = 0});
      m_vao->attribute({.index = 1, .size = 4, .type = GL_DOUBLE, .relativeOffset = offsetof(LineVertex, color)});
    }

    m_buffer->allocate(
      static_cast<GLsizeiptr>(m_vertices.size() * sizeof(LineVertex)),
      GL::Buffer::Usage::DynamicDraw,
      m_vertices.data()
    );

    m_vao->attachVertexBuffer(0, *m_buffer, 0, sizeof(LineVertex));
  }

  void Grid::render(const GL::Program &program, const Camera &camera) const {
    if (m_lineCount == 0) return;

    m_vao->bind();
    program.bind();
    program.uniform("u_viewProjection", camera.viewProjection());

    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_lineCount * 2));
  }


  double Grid::computeStep(const double baseStep, const double zoom) const {
    if (zoom <= 0.0) return baseStep;

    const double targetPixels = 28.0;
    const double worldTarget = targetPixels / zoom;
    const double ratio = worldTarget / baseStep;

    const double exponent = std::floor(std::log10(ratio));
    const double decade = std::pow(10.0, exponent);
    const double normalized = ratio / decade;

    double snapped = 1.0;
    if (normalized <= 1.0) snapped = 1.0;
    else if (normalized <= 2.0) snapped = 2.0;
    else if (normalized <= 5.0) snapped = 5.0;
    else snapped = 10.0;

    return baseStep * snapped * decade;
  }

  void Grid::buildLines(const Camera &camera) {
    m_vertices.clear();

    const double zoom = camera.zoom();
    const glm::dvec2 pos = camera.position();
    const double halfW = static_cast<double>(camera.width()) / (2.0 * zoom);
    const double halfH = static_cast<double>(camera.height()) / (2.0 * zoom);

    const glm::dvec2 minBounds = pos - glm::dvec2(halfW, halfH);
    const glm::dvec2 maxBounds = pos + glm::dvec2(halfW, halfH);

    const double minorStep = computeStep(m_baseStep, zoom);
    const int majorEvery = 5;

    const long long xStart = static_cast<long long>(std::floor(minBounds.x / minorStep));
    const long long xEnd   = static_cast<long long>(std::ceil(maxBounds.x / minorStep));
    const long long yStart = static_cast<long long>(std::floor(minBounds.y / minorStep));
    const long long yEnd   = static_cast<long long>(std::ceil(maxBounds.y / minorStep));

    // Vertical lines
    for (long long ix = xStart; ix <= xEnd; ++ix) {
      const double x = static_cast<double>(ix) * minorStep;
      const bool isAxis = (ix == 0);
      const bool isMajor = (ix % majorEvery == 0);
      const auto &color = isAxis ? m_yAxisColor : (isMajor ? m_majorColor : m_minorColor);

      m_vertices.push_back({{x, minBounds.y}, color});
      m_vertices.push_back({{x, maxBounds.y}, color});
    }

    // Horizontal lines
    for (long long iy = yStart; iy <= yEnd; ++iy) {
      const double y = static_cast<double>(iy) * minorStep;
      const bool isAxis = (iy == 0);
      const bool isMajor = (iy % majorEvery == 0);
      const auto &color = isAxis ? m_xAxisColor : (isMajor ? m_majorColor : m_minorColor);

      m_vertices.push_back({{minBounds.x, y}, color});
      m_vertices.push_back({{maxBounds.x, y}, color});
    }

    m_lineCount = m_vertices.size() / 2;
  }
}
