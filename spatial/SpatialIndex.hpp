#ifndef QADRA_SPATIAL_INDEX_HPP
#define QADRA_SPATIAL_INDEX_HPP

#include "BoxAabb.hpp"
#include "Handle.hpp"

#include <memory>

namespace Qadra::Spatial
{
  class SpatialIndex
  {
  public:
    SpatialIndex ();
    ~SpatialIndex ();

    SpatialIndex ( const SpatialIndex & ) = delete;
    SpatialIndex &operator= ( const SpatialIndex & ) = delete;

    SpatialIndex ( SpatialIndex && ) noexcept;
    SpatialIndex &operator= ( SpatialIndex && ) noexcept;

    void insert ( Core::Handle handle, const Math::BoxAABB &box );

    void remove ( Core::Handle handle );

    void update ( Core::Handle handle, const Math::BoxAABB &box );

    std::vector<Core::Handle> intersects ( const Math::BoxAABB &box ) const;

    std::vector<Core::Handle> within ( const Math::BoxAABB &box ) const;

    std::vector<Core::Handle> nearest ( const glm::dvec2 &point, std::size_t k ) const;

    void rebuild ( const std::vector<std::pair<Core::Handle, Math::BoxAABB>> &entries );

    void clear ();

    std::size_t count () const;

  private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
  };
} // namespace Qadra::Spatial

#endif // QADRA_SPATIAL_INDEX_HPP
