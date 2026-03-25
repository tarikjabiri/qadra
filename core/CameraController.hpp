#ifndef QADRA_CORE_CAMERA_CONTROLLER_HPP
#define QADRA_CORE_CAMERA_CONTROLLER_HPP

#include "Camera.hpp"

#include <Qt>
#include <glm/glm.hpp>

namespace Qadra::Core
{
  class CameraController
  {
  public:
    explicit CameraController ( Camera &camera );

    void mousePress ( const glm::dvec2 &position, Qt::MouseButton button );

    void mouseRelease ( Qt::MouseButton button );

    void mouseMove ( const glm::dvec2 &position );

    void wheel ( float delta, const glm::dvec2 &position );

    [[nodiscard]] bool isPanning () const { return m_panning; }

  private:
    Camera &m_camera;
    glm::dvec2 m_lastMousePosition{};
    bool m_panning = false;
  };
} // namespace Qadra::Core

#endif // QADRA_CORE_CAMERA_CONTROLLER_HPP
