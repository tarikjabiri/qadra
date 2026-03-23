#ifndef QADRA_BOX_OBB_HPP
#define QADRA_BOX_OBB_HPP

#include "BoxAabb.hpp"

#include <array>
#include <glm/glm.hpp>

namespace Qadra::Math
{
  class BoxOBB
  {
  public:
    BoxOBB ();

    BoxOBB ( const glm::dvec2 &center, const glm::dvec2 &extents, double angle );

    std::array<glm::dvec2, 4> corners () const;

    explicit operator BoxAABB () const;

  private:
    glm::dvec2 m_center;
    glm::dvec2 m_extents;
    double m_angle{};
  };
} // namespace Qadra::Math

#endif // QADRA_BOX_OBB_HPP
