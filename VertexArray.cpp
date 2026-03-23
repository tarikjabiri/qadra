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

  void VertexArray::attribute ( const Attribute &attr ) const noexcept
  {
    glEnableVertexArrayAttrib ( m_handle, attr.index );
    if ( attr.type == GL_DOUBLE )
    {
      glVertexArrayAttribLFormat ( m_handle, attr.index, attr.size, attr.type,
                                   attr.relativeOffset );
    }
    else
    {
      glVertexArrayAttribFormat ( m_handle, attr.index, attr.size, attr.type, attr.normalized,
                                  attr.relativeOffset );
    }
    glVertexArrayAttribBinding ( m_handle, attr.index, attr.bindingIndex );
  }

  void VertexArray::bindingDivisor ( const GLuint bindingIndex,
                                     const GLuint divisor ) const noexcept
  {
    glVertexArrayBindingDivisor ( m_handle, bindingIndex, divisor );
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
