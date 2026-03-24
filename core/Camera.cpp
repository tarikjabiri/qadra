#include "Camera.hpp"

#include <algorithm>

namespace Qadra::Core
{
  void Camera::pan ( const glm::dvec2 &delta )
  {
    m_position += delta / m_zoom;
    compute ();
  }

  void Camera::zoom ( const double factor, const glm::dvec2 &target )
  {
    const double zoom = m_zoom * factor;
    m_position += ( target - m_position ) * ( 1.0 - m_zoom / zoom );
    m_zoom = zoom;
    compute ();
  }

  void Camera::resize ( const int width, const int height )
  {
    m_width = width;
    m_height = height;
    compute ();
  }

  void Camera::resizePreserveViewportOrigin ( const int width, const int height )
  {
    const glm::dvec2 viewportOriginWorld = screenToWorld ( glm::dvec2 ( 0.0, 0.0 ) );

    m_width = width;
    m_height = height;

    const double hw = static_cast<double> ( m_width ) / ( 2.0 * m_zoom );
    const double hh = static_cast<double> ( m_height ) / ( 2.0 * m_zoom );

    m_position.x = viewportOriginWorld.x + hw;
    m_position.y = viewportOriginWorld.y - hh;
    compute ();
  }

  glm::dvec2 Camera::screenToWorld ( const glm::dvec2 &screen ) const
  {
    const double x = ( screen.x - static_cast<double> ( m_width ) / 2.0f ) / m_zoom + m_position.x;
    const double y = ( static_cast<double> ( m_height ) / 2.0f - screen.y ) / m_zoom + m_position.y;
    return { x, y };
  }

  glm::dvec2 Camera::worldToScreen ( const glm::dvec2 &world ) const
  {
    const double x = ( world.x - m_position.x ) * m_zoom + static_cast<double> ( m_width ) / 2.0;
    const double y = static_cast<double> ( m_height ) / 2.0 - ( world.y - m_position.y ) * m_zoom;
    return { x, y };
  }

  Math::BoxAABB Camera::viewportBox () const
  {
    const auto bottomLeft = screenToWorld ( glm::dvec2 ( 0.0, static_cast<double> ( m_height ) ) );
    const auto topRight = screenToWorld ( glm::dvec2 ( static_cast<double> ( m_width ), 0.0 ) );
    return Math::BoxAABB ( bottomLeft, topRight );
  }

  void Camera::compute ()
  {
    const double hw = ( static_cast<double> ( m_width ) / 2.0f ) / m_zoom;
    const double hh = ( static_cast<double> ( m_height ) / 2.0f ) / m_zoom;

    const glm::dmat4 projection = glm::ortho ( -hw, hw, -hh, hh );

    const glm::dmat4 view =
        glm::translate ( glm::dmat4 ( 1.0f ), glm::dvec3 ( -m_position, 0.0f ) );

    m_viewProjection = projection * view;
  }
} // namespace Qadra::Core
