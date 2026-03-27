#include "LWPolyline.hpp"

#include "math/Bulge.hpp"

#include <utility>

namespace
{
  void expandSegment ( Qadra::Math::BoxAABB &box, const Qadra::Entity::LWPolylineVertex &start,
                       const Qadra::Entity::LWPolylineVertex &end )
  {
    box.expand ( start.point );
    box.expand ( end.point );

    const auto arc = Qadra::Math::arcFromBulge ( start.point, end.point, start.bulge );
    if ( arc.has_value () ) box.merge ( arc->bbox () );
  }
} // namespace

namespace Qadra::Entity
{
  LWPolyline::LWPolyline ( const Core::Handle handle, const std::uint32_t renderKey,
                           LWPolylineRecord record )
      : Entity ( handle, EntityType::LWPolyline, renderKey ),
        m_vertices ( std::move ( record.vertices ) ), m_closed ( record.closed ),
        m_bbox ( computeBBox ( m_vertices, m_closed ) )
  {
  }

  Math::BoxAABB LWPolyline::bbox () const { return m_bbox; }

  Math::BoxAABB LWPolyline::computeBBox ( const std::vector<LWPolylineVertex> &vertices,
                                          const bool closed )
  {
    Math::BoxAABB box;
    if ( vertices.empty () ) return box;

    box.expand ( vertices.front ().point );

    for ( std::size_t index = 0; index + 1 < vertices.size (); ++index )
      expandSegment ( box, vertices[index], vertices[index + 1] );

    if ( closed && vertices.size () > 1 )
      expandSegment ( box, vertices.back (), vertices.front () );

    return box;
  }
} // namespace Qadra::Entity
