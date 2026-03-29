#ifndef QADRA_RENDER_MANAGED_BUFFER_HPP
#define QADRA_RENDER_MANAGED_BUFFER_HPP

#include "SlotAllocator.hpp"
#include "gl/Buffer.hpp"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace Qadra::Render
{
  template <typename T>
  class ManagedBuffer
  {
    static_assert ( std::is_trivially_copyable_v<T> );

  public:
    enum class WriteMode
    {
      Immediate,
      Deferred,
    };

    explicit ManagedBuffer ( const GL::Buffer::Target target = GL::Buffer::Target::ArrayBuffer,
                             const std::size_t initialCapacity = 4096 )
        : m_buffer ( target ), m_target ( target ), m_allocator ( initialCapacity ),
          m_cpuMirror ( initialCapacity )
    {
    }

    ~ManagedBuffer () noexcept { releaseMapping (); }

    ManagedBuffer ( const ManagedBuffer & ) = delete;
    ManagedBuffer &operator= ( const ManagedBuffer & ) = delete;

    ManagedBuffer ( ManagedBuffer && ) = delete;
    ManagedBuffer &operator= ( ManagedBuffer && ) = delete;

    [[nodiscard]] SlotAllocator::Range insert ( const std::span<const T> items,
                                                const WriteMode mode = WriteMode::Immediate )
    {
      if ( items.empty () ) return {};

      const auto range = m_allocator.allocate ( static_cast<std::uint32_t> ( items.size () ) );
      ensureCapacity ();
      writeMirror ( range, items );
      if ( mode == WriteMode::Immediate ) uploadRange ( range );
      return range;
    }

    void update ( const SlotAllocator::Range range, const std::span<const T> items,
                  const WriteMode mode = WriteMode::Immediate )
    {
      if ( range.empty () ) return;

      writeMirror ( range, items );
      if ( mode == WriteMode::Immediate ) uploadRange ( range );
    }

    void erase ( const SlotAllocator::Range range, const WriteMode mode = WriteMode::Immediate,
                 const bool releaseSlots = true )
    {
      if ( range.empty () ) return;

      ensureMirrorSize ();
      std::fill_n ( m_cpuMirror.begin () + range.first, range.count, T{} );
      if ( mode == WriteMode::Immediate ) uploadRange ( range );
      if ( releaseSlots ) m_allocator.release ( range );
    }

    void uploadAll ()
    {
      const std::size_t count = m_allocator.highWater ();
      if ( count == 0 || m_mappedData == nullptr ) return;

      std::memcpy ( m_mappedData, m_cpuMirror.data (), count * sizeof ( T ) );
    }

    void clear () { m_allocator.clear (); }

    [[nodiscard]] const GL::Buffer &buffer () const noexcept { return m_buffer; }

    [[nodiscard]] std::size_t highWater () const noexcept { return m_allocator.highWater (); }

    [[nodiscard]] std::size_t usedSlots () const noexcept { return m_allocator.usedSlots (); }

    [[nodiscard]] float fragmentation () const noexcept { return m_allocator.fragmentation (); }

  private:
    static constexpr GLbitfield kStorageFlags =
        GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

    void ensureCapacity ()
    {
      const std::size_t required = m_allocator.capacity ();
      if ( required <= m_gpuCapacity )
      {
        ensureMirrorSize ();
        return;
      }

      ensureMirrorSize ();
      reallocateGpuStorage ( required );
    }

    void ensureMirrorSize ()
    {
      if ( m_cpuMirror.size () < m_allocator.capacity () )
        m_cpuMirror.resize ( m_allocator.capacity () );
    }

    void reallocateGpuStorage ( const std::size_t capacity )
    {
      releaseMapping ();

      GL::Buffer nextBuffer ( m_target );
      nextBuffer.allocateStorage ( static_cast<GLsizeiptr> ( capacity * sizeof ( T ) ),
                                   kStorageFlags );
      auto *mapped = static_cast<T *> ( nextBuffer.mapRange (
          0, static_cast<GLsizeiptr> ( capacity * sizeof ( T ) ), kStorageFlags ) );
      if ( mapped == nullptr )
        throw std::runtime_error ( "ManagedBuffer: failed to map persistent buffer." );

      const std::size_t prefix = std::min ( m_allocator.highWater (), m_cpuMirror.size () );
      if ( prefix > 0 ) std::memcpy ( mapped, m_cpuMirror.data (), prefix * sizeof ( T ) );

      m_buffer = std::move ( nextBuffer );
      m_mappedData = mapped;
      m_gpuCapacity = capacity;
    }

    void writeMirror ( const SlotAllocator::Range range, const std::span<const T> items )
    {
      if ( items.size () != range.count )
        throw std::runtime_error ( "ManagedBuffer: range size mismatch." );

      std::memcpy ( m_cpuMirror.data () + range.first, items.data (), items.size_bytes () );
    }

    void uploadRange ( const SlotAllocator::Range range )
    {
      if ( range.empty () || m_mappedData == nullptr ) return;

      std::memcpy ( m_mappedData + range.first, m_cpuMirror.data () + range.first,
                    range.count * sizeof ( T ) );
    }

    void releaseMapping () noexcept
    {
      if ( m_mappedData == nullptr ) return;

      static_cast<void> ( m_buffer.unmap () );
      m_mappedData = nullptr;
    }

    GL::Buffer m_buffer;
    GL::Buffer::Target m_target;
    SlotAllocator m_allocator;
    std::vector<T> m_cpuMirror;
    T *m_mappedData = nullptr;
    std::size_t m_gpuCapacity = 0;
  };
} // namespace Qadra::Render

#endif // QADRA_RENDER_MANAGED_BUFFER_HPP
