#include "CameraController.hpp"

#include <cmath>

namespace Qadra::Core
{
  CameraController::CameraController ( Camera &camera ) : m_camera ( camera ) { }

  void CameraController::mousePress ( const glm::dvec2 &position, Qt::MouseButton button )
  {
    if ( button == Qt::MiddleButton )
    {
      m_panning = true;
      m_lastMousePosition = position;
    }
  }

  void CameraController::mouseRelease ( const Qt::MouseButton button )
  {
    if ( button == Qt::MiddleButton )
    {
      m_panning = false;
    }
  }

  void CameraController::mouseMove ( const glm::dvec2 &position )
  {
    if ( ! m_panning ) return;

    const glm::dvec2 delta = position - m_lastMousePosition;
    m_camera.pan ( glm::dvec2 ( -delta.x, delta.y ) );
    m_lastMousePosition = position;
  }

  void CameraController::wheel ( const float delta, const glm::dvec2 &position )
  {
    const glm::dvec2 mouseWorld = m_camera.screenToWorld ( position );
    m_camera.zoom ( std::pow ( 1.5, delta ), mouseWorld );
  }
} // namespace Qadra::Core
