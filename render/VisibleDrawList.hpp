#ifndef QADRA_VISIBLE_DRAW_LIST_HPP
#define QADRA_VISIBLE_DRAW_LIST_HPP

#include <glad/gl.h>
#include <vector>

namespace Qadra::Render
{
  class VisibleDrawList
  {
  public:
    void clear ();

    void append ( GLint first, GLsizei count );

    [[nodiscard]] bool empty () const noexcept { return m_firsts.empty (); }

    [[nodiscard]] std::size_t size () const noexcept { return m_firsts.size (); }

    [[nodiscard]] const std::vector<GLint> &firsts () const noexcept { return m_firsts; }

    [[nodiscard]] const std::vector<GLsizei> &counts () const noexcept { return m_counts; }

  private:
    std::vector<GLint> m_firsts;
    std::vector<GLsizei> m_counts;
  };
} // namespace Qadra::Render

#endif // QADRA_VISIBLE_DRAW_LIST_HPP
