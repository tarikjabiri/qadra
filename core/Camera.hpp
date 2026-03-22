#ifndef QADRA_CORE_CAMERA_HPP
#define QADRA_CORE_CAMERA_HPP

#include <glm/gtc/matrix_transform.hpp>

namespace Qadra::Core {
  class Camera {
  public:
    Camera() = default;

    void pan(const glm::dvec2 &delta);

    void zoom(double factor, const glm::dvec2 &target);

    void resize(int width, int height);

    glm::dvec2 screenToWorld(const glm::dvec2 &screen) const;

    [[nodiscard]] const glm::dmat4 &viewProjection() const { return m_viewProjection; }

    [[nodiscard]] double zoom() const { return m_zoom; }

    [[nodiscard]] const glm::dvec2 &position() const { return m_position; }

    [[nodiscard]] int width() const { return m_width; }

    [[nodiscard]] int height() const { return m_height; }

  private:
    void compute();

    glm::dvec2 m_position{0.0f};
    double m_zoom{1.0f};
    int m_width{800};
    int m_height{600};
    glm::dmat4 m_viewProjection{1.0f};
  };
} // Qadra::Core

#endif //QADRA_CORE_CAMERA_HPP
