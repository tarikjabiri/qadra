#ifndef QADRA_ENTITY_LW_POLYLINE_HPP
#define QADRA_ENTITY_LW_POLYLINE_HPP

#include "Entity.hpp"

#include <cstdint>
#include <vector>

namespace Qadra::Entity
{
  struct LWPolylineVertex
  {
    glm::dvec2 point{ 0.0 };
    double bulge = 0.0;
  };

  struct LWPolylineRecord
  {
    std::vector<LWPolylineVertex> vertices;
    bool closed = false;
  };

  class LWPolyline : public Entity
  {
  public:
    LWPolyline ( Core::Handle handle, std::uint32_t renderKey, LWPolylineRecord record );

    [[nodiscard]] const std::vector<LWPolylineVertex> &vertices () const noexcept
    {
      return m_vertices;
    }

    [[nodiscard]] bool closed () const noexcept { return m_closed; }

    [[nodiscard]] Math::BoxAABB bbox () const override;

  private:
    static Math::BoxAABB computeBBox ( const std::vector<LWPolylineVertex> &vertices, bool closed );

    std::vector<LWPolylineVertex> m_vertices;
    bool m_closed = false;
    Math::BoxAABB m_bbox;
  };
} // namespace Qadra::Entity

#endif // QADRA_ENTITY_LW_POLYLINE_HPP
