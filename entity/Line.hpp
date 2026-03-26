#ifndef QADRA_LINE_HPP
#define QADRA_LINE_HPP

#include "Entity.hpp"

#include <cstdint>

namespace Qadra::Entity
{
  struct LineRecord
  {
    glm::dvec2 start;
    glm::dvec2 end;
  };

  class Line : public Entity
  {
  public:
    Line ( Core::Handle handle, std::uint32_t renderKey, const LineRecord &record );

    const glm::dvec2 &start () const { return m_start; }

    const glm::dvec2 &end () const { return m_end; }

    Math::BoxAABB bbox () const override;

  private:
    glm::dvec2 m_start;
    glm::dvec2 m_end;
  };
} // namespace Qadra::Entity

#endif // QADRA_LINE_HPP
