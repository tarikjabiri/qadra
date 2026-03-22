#include "Camera.hpp"

#include <algorithm>

namespace Qadra::Core {
  void Camera::pan(const glm::dvec2 &delta) {
    m_position += delta / m_zoom;
    compute();
  }

  void Camera::zoom(const double factor, const glm::dvec2& target) {
    const double zoom = m_zoom * factor;
    m_position += (target - m_position) * (1.0 - m_zoom / zoom);
    m_zoom = zoom;
    compute();
  }

  void Camera::resize(const int width, const int height) {
    m_width = width;
    m_height = height;
    compute();
  }

  glm::dvec2 Camera::screenToWorld(const glm::dvec2 &screen) const {
    const double x = (screen.x - static_cast<double>(m_width) / 2.0f) / m_zoom + m_position.x;
    const double y = (static_cast<double>(m_height) / 2.0f - screen.y) / m_zoom + m_position.y;
    return {x, y};
  }

  void Camera::compute() {
    const double halfWidth = (static_cast<double>(m_width) / 2.0f) / m_zoom;
    const double halfHeight = (static_cast<double>(m_height) / 2.0f) / m_zoom;

    const glm::dmat4 projection = glm::ortho(-halfWidth, halfWidth,-halfHeight, halfHeight);

    const glm::dmat4 view = glm::translate(glm::dmat4(1.0f), glm::dvec3(-m_position, 0.0f));

    m_viewProjection = projection * view;
  }
} // Qadra::Core
