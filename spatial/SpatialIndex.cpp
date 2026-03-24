#include "SpatialIndex.hpp"

#include "BoxAabb.hpp"
#include "Handle.hpp"

#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <ranges>
#include <unordered_map>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

namespace Qadra::Spatial
{
  using BPoint = bg::model::point<double, 2, bg::cs::cartesian>;
  using BBox = bg::model::box<BPoint>;
  using Entry = std::pair<BBox, Core::Handle>;

  namespace
  {
    BBox toBBox ( const Math::BoxAABB &box )
    {
      return BBox ( BPoint ( box.min ().x, box.min ().y ), BPoint ( box.max ().x, box.max ().y ) );
    }

    BPoint toBPoint ( const glm::dvec2 &point ) { return BPoint ( point.x, point.y ); }

    std::vector<Core::Handle> extractHandles ( const std::vector<Entry> &results )
    {
      std::vector<Core::Handle> handles;
      handles.reserve ( results.size () );
      for ( const auto &handle : results | std::views::values ) handles.push_back ( handle );
      return handles;
    }
  } // namespace

  struct SpatialIndex::Impl
  {
    bgi::rtree<Entry, bgi::rstar<16>> tree;
    std::unordered_map<Core::Handle, BBox> lookup;
  };

  SpatialIndex::SpatialIndex () : m_impl ( std::make_unique<Impl> () ) { }

  SpatialIndex::~SpatialIndex () = default;

  SpatialIndex::SpatialIndex ( SpatialIndex && ) noexcept = default;

  SpatialIndex &SpatialIndex::operator= ( SpatialIndex && ) noexcept = default;

  void SpatialIndex::insert ( Core::Handle handle, const Math::BoxAABB &box )
  {
    const auto bbox = toBBox ( box );
    m_impl->tree.insert ( { bbox, handle } );
    m_impl->lookup.emplace ( handle, bbox );
  }

  void SpatialIndex::remove ( Core::Handle handle )
  {
    const auto it = m_impl->lookup.find ( handle );
    if ( it == m_impl->lookup.end () ) return;

    m_impl->tree.remove ( { it->second, handle } );
    m_impl->lookup.erase ( it );
  }

  void SpatialIndex::update ( const Core::Handle handle, const Math::BoxAABB &box )
  {
    remove ( handle );
    insert ( handle, box );
  }

  std::vector<Core::Handle> SpatialIndex::intersects ( const Math::BoxAABB &box ) const
  {
    std::vector<Entry> results;
    m_impl->tree.query ( bgi::intersects ( toBBox ( box ) ), std::back_inserter ( results ) );

    return extractHandles ( results );
  }

  std::vector<Core::Handle> SpatialIndex::within ( const Math::BoxAABB &box ) const
  {
    std::vector<Entry> results;
    m_impl->tree.query ( bgi::within ( toBBox ( box ) ), std::back_inserter ( results ) );

    return extractHandles ( results );
  }

  std::vector<Core::Handle> SpatialIndex::nearest ( const glm::dvec2 &point,
                                                    const std::size_t k ) const
  {
    std::vector<Entry> results;
    m_impl->tree.query ( bgi::nearest ( toBPoint ( point ), k ), std::back_inserter ( results ) );

    return extractHandles ( results );
  }

  void SpatialIndex::rebuild ( const std::vector<std::pair<Core::Handle, Math::BoxAABB>> &entries )
  {
    m_impl->lookup.clear ();

    std::vector<Entry> packed;
    packed.reserve ( entries.size () );
    for ( const auto &[handle, box] : entries )
    {
      auto bbox = toBBox ( box );
      packed.emplace_back ( bbox, handle );
      m_impl->lookup.emplace ( handle, bbox );
    }

    m_impl->tree = bgi::rtree<Entry, bgi::rstar<16>> ( packed.begin (), packed.end () );
  }

  void SpatialIndex::clear ()
  {
    m_impl->tree.clear ();
    m_impl->lookup.clear ();
  }

  std::size_t SpatialIndex::count () const { return m_impl->tree.size (); }
} // namespace Qadra::Spatial
