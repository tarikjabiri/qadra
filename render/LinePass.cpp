#include "LinePass.hpp"

#include <stdexcept>

namespace Qadra::Render
{
  void LinePass::init ( const QString &vertexSource, const QString &fragmentSource )
  {
    GL::Shader vertexShader ( GL::Shader::Type::Vertex );
    GL::Shader fragmentShader ( GL::Shader::Type::Fragment );

    if ( ! vertexShader.compile ( vertexSource ) || ! fragmentShader.compile ( fragmentSource ) )
      throw std::runtime_error ( "LinePass: shader compilation failed" );

    if ( ! m_program.link ( vertexShader, fragmentShader ) )
      throw std::runtime_error ( "LinePass: program linking failed" );

    m_vao.attribute ( { .index = 0,
                        .size = 2,
                        .type = GL_DOUBLE,
                        .relativeOffset = offsetof ( Vertex, position ) } );
    m_vao.attribute (
        { .index = 1, .size = 4, .type = GL_FLOAT, .relativeOffset = offsetof ( Vertex, color ) } );
    m_vao.attribute (
        { .index = 2, .size = 1, .type = GL_FLOAT, .relativeOffset = offsetof ( Vertex, depth ) } );
  }

  void LinePass::upload ( const std::span<const Vertex> vertices )
  {
    if ( vertices.empty () )
    {
      m_vertexCount = 0;
      m_bufferDirty = false;
      return;
    }

    m_vbo.allocate ( vertices, GL::Buffer::Usage::StaticDraw );
    m_vao.attachVertexBuffer ( 0, m_vbo, 0, sizeof ( Vertex ) );
    m_vertexCount = vertices.size ();
    m_bufferDirty = false;
  }

  void LinePass::render ( const Core::Camera &camera ) const
  {
    if ( m_vertexCount == 0 ) return;

    m_vao.bind ();
    m_program.bind ();
    m_program.uniform ( "u_viewProjection", camera.viewProjection () );

    glDrawArrays ( GL_LINES, 0, static_cast<GLsizei> ( m_vertexCount ) );
  }
} // namespace Qadra::Render
