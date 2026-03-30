#include "VertexArray.hpp"

namespace Qadra::GL
{
  VertexArray::VertexArray () { glCreateVertexArrays ( 1, &m_handle ); }

  VertexArray::~VertexArray () noexcept { glDeleteVertexArrays ( 1, &m_handle ); }

  VertexArray::VertexArray ( VertexArray &&rhs ) noexcept : m_handle ( rhs.m_handle )
  {
    rhs.m_handle = 0;
  }

  VertexArray &VertexArray::operator= ( VertexArray &&rhs ) noexcept
  {
    if ( this != &rhs )
    {
      glDeleteVertexArrays ( 1, &m_handle );
      m_handle = rhs.m_handle;
      rhs.m_handle = 0;
    }

    return *this;
  }

  void VertexArray::bind () const noexcept { glBindVertexArray ( m_handle ); }

  void VertexArray::unbind () noexcept { glBindVertexArray ( 0 ); }

  void VertexArray::attribute ( const VertexAttribute &attr ) const noexcept
  {
    glEnableVertexArrayAttrib ( m_handle, attr.index );
    if ( attr.type == GL_DOUBLE )
    {
      glVertexArrayAttribLFormat ( m_handle, attr.index, attr.size, attr.type,
                                   attr.relativeOffset );
    }
    else if ( attr.integer )
    {
      glVertexArrayAttribIFormat ( m_handle, attr.index, attr.size, attr.type,
                                   attr.relativeOffset );
    }
    else
    {
      glVertexArrayAttribFormat ( m_handle, attr.index, attr.size, attr.type, attr.normalized,
                                  attr.relativeOffset );
    }
    glVertexArrayAttribBinding ( m_handle, attr.index, attr.bindingIndex );
  }

  void VertexArray::attributes ( const std::span<const VertexAttribute> attrs ) const noexcept
  {
    for ( const VertexAttribute &attr : attrs ) attribute ( attr );
  }

  void VertexArray::bindingDivisor ( const GLuint bindingIndex,
                                     const GLuint divisor ) const noexcept
  {
    glVertexArrayBindingDivisor ( m_handle, bindingIndex, divisor );
  }

  void VertexArray::bindings ( const std::span<const VertexBinding> bindings ) const noexcept
  {
    for ( const VertexBinding &binding : bindings )
      bindingDivisor ( binding.bindingIndex, binding.divisor );
  }

  void VertexArray::applyLayout ( const VertexLayout &layout ) const noexcept
  {
    attributes ( layout.attributes () );
    bindings ( layout.bindings () );
  }

  void VertexArray::attachVertexBuffer ( const GLuint bindingIndex, const Buffer &buffer,
                                         const GLintptr offset, const GLsizei stride ) const
  {
    glVertexArrayVertexBuffer ( m_handle, bindingIndex, buffer.handle (), offset, stride );
  }

  void VertexArray::attachElementBuffer ( const Buffer &buffer ) const
  {
    glVertexArrayElementBuffer ( m_handle, buffer.handle () );
  }

  void swap ( VertexArray &lhs, VertexArray &rhs ) noexcept
  {
    std::swap ( lhs.m_handle, rhs.m_handle );
  }
} // namespace Qadra::GL
