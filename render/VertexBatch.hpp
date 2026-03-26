#ifndef QADRA_VERTEX_BATCH_HPP
#define QADRA_VERTEX_BATCH_HPP

#include "gl/Buffer.hpp"

#include <algorithm>
#include <span>

namespace Qadra::Render
{
  template <typename V>
  class VertexBatch
  {
  public:
    void upload ( std::span<const V> vertices,
                  const GL::Buffer::Usage usage = GL::Buffer::Usage::DynamicDraw )
    {
      if ( vertices.empty () )
      {
        m_vertexCount = 0;
        return;
      }

      reserve ( vertices.size (), usage );
      m_buffer.update ( 0, vertices );
      m_vertexCount = vertices.size ();
    }

    [[nodiscard]] const GL::Buffer &buffer () const noexcept { return m_buffer; }

    [[nodiscard]] std::size_t vertexCount () const noexcept { return m_vertexCount; }

  private:
    void reserve ( const std::size_t requiredVertices, const GL::Buffer::Usage usage )
    {
      if ( requiredVertices <= m_capacityVertices ) return;

      std::size_t nextCapacity = std::max<std::size_t> ( 256, m_capacityVertices );
      while ( nextCapacity < requiredVertices ) nextCapacity *= 2;

      m_buffer.allocate ( static_cast<GLsizeiptr> ( nextCapacity * sizeof ( V ) ), usage, nullptr );
      m_capacityVertices = nextCapacity;
    }

    GL::Buffer m_buffer;
    std::size_t m_vertexCount = 0;
    std::size_t m_capacityVertices = 0;
  };
} // namespace Qadra::Render

#endif // QADRA_VERTEX_BATCH_HPP
