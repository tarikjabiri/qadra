#ifndef QADRA_BOX_AABB_HPP
#define QADRA_BOX_AABB_HPP

#include <glm/glm.hpp>

namespace Qadra::Math
{
  class BoxAABB
  {
  public:
    BoxAABB ();
    BoxAABB ( const glm::dvec2 &first, const glm::dvec2 &second );

    glm::dvec2 center () const { return ( m_min + m_max ) * 0.5; }

    glm::dvec2 size () const { return m_max - m_min; }

    bool contains ( const glm::dvec2 &point ) const
    {
      return point.x >= m_min.x && point.x <= m_max.x && point.y >= m_min.y && point.y <= m_max.y;
    }

    bool intersects ( const BoxAABB &other ) const
    {
      return m_min.x <= other.m_max.x && m_max.x >= other.m_min.x && m_min.y <= other.m_max.y &&
             m_max.y >= other.m_min.y;
    }

    void expand ( const glm::dvec2 &point )
    {
      m_min = glm::min ( m_min, point );
      m_max = glm::max ( m_max, point );
    }

    void merge ( const BoxAABB &other )
    {
      m_min = glm::min ( m_min, other.m_min );
      m_max = glm::max ( m_max, other.m_max );
    }

    BoxAABB expanded ( const double margin ) const
    {
      return BoxAABB ( m_min - glm::dvec2 ( margin ), m_max + glm::dvec2 ( margin ) );
    }

    [[nodiscard]] glm::dvec2 min () const { return m_min; }

    [[nodiscard]] glm::dvec2 max () const { return m_max; }

  private:
    glm::dvec2 m_min;
    glm::dvec2 m_max;
  };
} // namespace Qadra::Math

#endif // QADRA_BOX_AABB_HPP
