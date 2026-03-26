#include "VisibleDrawList.hpp"

namespace Qadra::Render
{
  void VisibleDrawList::clear ()
  {
    m_firsts.clear ();
    m_counts.clear ();
  }

  void VisibleDrawList::append ( const GLint first, const GLsizei count )
  {
    if ( count <= 0 ) return;

    if ( ! m_firsts.empty () )
    {
      const auto lastIndex = m_firsts.size () - 1;
      if (const auto lastEnd = m_firsts[lastIndex] + m_counts[lastIndex]; lastEnd == first )
      {
        m_counts[lastIndex] += count;
        return;
      }
    }

    m_firsts.push_back ( first );
    m_counts.push_back ( count );
  }
} // namespace Qadra::Render
