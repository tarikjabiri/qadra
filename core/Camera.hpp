#ifndef QADRA_CORE_CAMERA_HPP
#define QADRA_CORE_CAMERA_HPP

#include "BoxAabb.hpp"

#include <QPointF>
#include <glm/gtc/matrix_transform.hpp>

namespace Qadra::Core
{
  class Camera
  {
  public:
    static constexpr double kMinZoom = 1e-6;
    static constexpr double kMaxZoom = 1e+8;

    Camera () = default;

    void pan ( const glm::dvec2 &delta );

    void zoom ( double factor, const glm::dvec2 &target );

    void resize ( int width, int height );

    void resizePreserveViewportOrigin ( int width, int height );

    void fitToBox ( const Math::BoxAABB &box, double padding = 1.1 );

    void setDevicePixelRatio ( const double ratio ) { m_devicePixelRatio = ratio; }

    void fitToScreenRect ( const glm::dvec2 &screenMin, const glm::dvec2 &screenMax );

    [[nodiscard]] glm::dvec2 screenToWorld ( const glm::dvec2 &screen ) const;

    [[nodiscard]] glm::dvec2 worldToScreen ( const glm::dvec2 &world ) const;

    [[nodiscard]] const glm::dmat4 &viewProjection () const { return m_viewProjection; }

    [[nodiscard]] double zoom () const { return m_zoom; }

    [[nodiscard]] const glm::dvec2 &position () const { return m_position; }

    [[nodiscard]] int width () const { return m_width; }

    [[nodiscard]] int height () const { return m_height; }

    [[nodiscard]] Math::BoxAABB viewportBox () const;

    [[nodiscard]] double pixelSizeInWorld () const { return 1.0 / m_zoom; }

    [[nodiscard]] glm::dvec2 viewportSizeWorld () const;

    [[nodiscard]] int devicePixels ( int logical ) const;

    [[nodiscard]] glm::dvec2 devicePixels ( const QPointF &logical ) const;

  private:
    void compute ();

    glm::dvec2 m_position{ 0.0f };
    double m_zoom{ 1.0f };
    int m_width{ 800 };
    int m_height{ 600 };
    glm::dmat4 m_viewProjection{ 1.0f };
    double m_devicePixelRatio{ 1 };
  };
} // namespace Qadra::Core

#endif // QADRA_CORE_CAMERA_HPP
