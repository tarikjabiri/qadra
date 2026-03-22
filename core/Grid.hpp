#ifndef QADRA_GRID_HPP
#define QADRA_GRID_HPP

#include <optional>

#include "Camera.hpp"
#include "Program.hpp"
#include "VertexArray.hpp"

namespace Qadra::Core {
  class Grid {
  public:
    Grid();

    void update(const Camera &camera);

    void render(const GL::Program &program, const Camera &camera) const;

  private:
    struct LineVertex {
      glm::dvec2 position;
      glm::dvec4 color;
    };

    double computeStep(double baseStep, double zoom) const;

    void buildLines(const Camera &camera);

    double m_baseStep{1.0};
    double m_pixelThreshold{10.0};

    glm::dvec4 m_minorColor{0.14, 0.15, 0.17, 1.0};
    glm::dvec4 m_majorColor{0.28, 0.30, 0.34, 1.0};
    glm::dvec4 m_xAxisColor{0.75, 0.30, 0.30, 1.0};
    glm::dvec4 m_yAxisColor{0.30, 0.75, 0.30, 1.0};

    std::vector<LineVertex> m_vertices;
    std::optional<GL::Buffer> m_buffer;
    std::optional<GL::VertexArray> m_vao;
    std::size_t m_lineCount{0};
  };
} // Qadra::Core

#endif //QADRA_GRID_HPP
