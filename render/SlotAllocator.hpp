#ifndef QADRA_RENDER_SLOT_ALLOCATOR_HPP
#define QADRA_RENDER_SLOT_ALLOCATOR_HPP

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace Qadra::Render
{
  class SlotAllocator
  {
  public:
    struct Range
    {
      std::uint32_t first = 0;
      std::uint32_t count = 0;

      [[nodiscard]] bool empty () const noexcept { return count == 0; }
    };

    explicit SlotAllocator ( const std::size_t initialCapacity = 4096 )
        : m_capacity ( initialCapacity )
    {
    }

    [[nodiscard]] Range allocate ( const std::uint32_t count )
    {
      if ( count == 0 ) return {};

      auto bestIt = m_freeRanges.end ();
      for ( auto it = m_freeRanges.begin (); it != m_freeRanges.end (); ++it )
      {
        if ( it->count < count ) continue;
        if ( bestIt == m_freeRanges.end () || it->count < bestIt->count ) bestIt = it;
        if ( it->count == count ) break;
      }

      if ( bestIt != m_freeRanges.end () )
      {
        const Range result{ bestIt->first, count };
        if ( bestIt->count == count ) m_freeRanges.erase ( bestIt );
        else
        {
          bestIt->first += count;
          bestIt->count -= count;
        }

        m_usedSlots += count;
        return result;
      }

      if ( m_highWater + count > m_capacity )
        m_capacity =
            std::max ( std::max<std::size_t> ( 256, m_capacity * 2 ), m_highWater + count );

      const Range result{ static_cast<std::uint32_t> ( m_highWater ), count };
      m_highWater += count;
      m_usedSlots += count;
      return result;
    }

    void release ( const Range range )
    {
      if ( range.empty () ) return;

      m_usedSlots -= range.count;
      m_freeRanges.push_back ( range );
      mergeAdjacentFreeRanges ();
    }

    void clear ()
    {
      m_highWater = 0;
      m_usedSlots = 0;
      m_freeRanges.clear ();
    }

    [[nodiscard]] std::size_t highWater () const noexcept { return m_highWater; }

    [[nodiscard]] std::size_t capacity () const noexcept { return m_capacity; }

    [[nodiscard]] std::size_t usedSlots () const noexcept { return m_usedSlots; }

    [[nodiscard]] float fragmentation () const noexcept
    {
      if ( m_highWater == 0 ) return 0.0f;

      return 1.0f - static_cast<float> ( m_usedSlots ) / static_cast<float> ( m_highWater );
    }

  private:
    void mergeAdjacentFreeRanges ()
    {
      if ( m_freeRanges.size () < 2 ) return;

      std::sort ( m_freeRanges.begin (), m_freeRanges.end (),
                  [] ( const Range &lhs, const Range &rhs ) { return lhs.first < rhs.first; } );

      std::vector<Range> merged;
      merged.reserve ( m_freeRanges.size () );
      merged.push_back ( m_freeRanges.front () );

      for ( std::size_t i = 1; i < m_freeRanges.size (); ++i )
      {
        Range &tail = merged.back ();
        if ( tail.first + tail.count == m_freeRanges[i].first ) tail.count += m_freeRanges[i].count;
        else
          merged.push_back ( m_freeRanges[i] );
      }

      if ( ! merged.empty () )
      {
        const Range &tail = merged.back ();
        if ( tail.first + tail.count == static_cast<std::uint32_t> ( m_highWater ) )
        {
          m_highWater = tail.first;
          merged.pop_back ();
        }
      }

      m_freeRanges = std::move ( merged );
    }

    std::size_t m_capacity = 0;
    std::size_t m_highWater = 0;
    std::size_t m_usedSlots = 0;
    std::vector<Range> m_freeRanges;
  };
} // namespace Qadra::Render

#endif // QADRA_RENDER_SLOT_ALLOCATOR_HPP
