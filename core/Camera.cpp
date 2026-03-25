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
    const double zoom = std::clamp ( m_zoom * factor, kMinZoom, kMaxZoom );
    if ( zoom == m_zoom ) return;

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

  void Camera::fitToBox ( const Math::BoxAABB &box, double padding )
  {
    const glm::dvec2 size = box.max () - box.min ();
    if ( size.x <= 0.0 || size.y <= 0.0 ) return;

    m_position = ( box.min () + box.max () ) * 0.5;
    m_zoom = std::min ( static_cast<double> ( m_width ) / size.x,
                        static_cast<double> ( m_height ) / size.y ) /
             padding;
    m_zoom = std::clamp ( m_zoom, kMinZoom, kMaxZoom );
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

  glm::dvec2 Camera::viewportSizeWorld () const
  {
    return { static_cast<double> ( m_width ) / m_zoom, static_cast<double> ( m_height ) / m_zoom };
  }

  int Camera::devicePixels ( int logical ) const
  {
    return static_cast<int> (
        std::lround ( static_cast<double> ( logical ) * m_devicePixelRatio ) );
  }

  glm::dvec2 Camera::devicePixels ( const QPointF &logical ) const
  {
    return { logical.x () * m_devicePixelRatio, logical.y () * m_devicePixelRatio };
  }

  void Camera::fitToScreenRect ( const glm::dvec2 &screenMin, const glm::dvec2 &screenMax )
  {
    const glm::dvec2 worldMin = screenToWorld ( screenMin );
    const glm::dvec2 worldMax = screenToWorld ( screenMax );

    const glm::dvec2 lo = glm::min ( worldMin, worldMax );
    const glm::dvec2 hi = glm::max ( worldMin, worldMax );

    fitToBox ( Math::BoxAABB ( lo, hi ) );
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
