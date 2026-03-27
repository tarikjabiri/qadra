#include "LWPolylineGeometryBuilder.hpp"

#include "ArcInstanceBuilder.hpp"
#include "math/Bulge.hpp"

namespace
{
  [[nodiscard]] bool samePoint ( const glm::dvec2 &lhs, const glm::dvec2 &rhs ) noexcept
  {
    const glm::dvec2 delta = lhs - rhs;
    return glm::dot ( delta, delta ) <= 1e-18;
  }

  void appendSegment ( Qadra::Render::LWPolylineGeometry &geometry,
                       const Qadra::Entity::LWPolylineVertex &start,
                       const Qadra::Entity::LWPolylineVertex &end, const glm::vec4 &color,
                       const std::uint32_t renderKey )
  {
    if ( samePoint ( start.point, end.point ) ) return;

    const auto arc = Qadra::Math::arcFromBulge ( start.point, end.point, start.bulge );
    if ( arc.has_value () )
    {
      geometry.arcInstances.push_back (
          Qadra::Render::buildArcInstance ( *arc, color, renderKey ) );
      return;
    }

    geometry.lineVertices.push_back ( { start.point, color, renderKey } );
    geometry.lineVertices.push_back ( { end.point, color, renderKey } );
  }
} // namespace

namespace Qadra::Render
{
  LWPolylineGeometry buildLWPolylineGeometry ( const Entity::LWPolyline &lwPolyline,
                                               const glm::vec4 &color,
                                               const std::uint32_t renderKey )
  {
    LWPolylineGeometry geometry;
    const auto &vertices = lwPolyline.vertices ();
    if ( vertices.size () < 2 ) return geometry;

    for ( std::size_t index = 0; index + 1 < vertices.size (); ++index )
      appendSegment ( geometry, vertices[index], vertices[index + 1], color, renderKey );

    if ( lwPolyline.closed () )
      appendSegment ( geometry, vertices.back (), vertices.front (), color, renderKey );

    return geometry;
  }
} // namespace Qadra::Render
