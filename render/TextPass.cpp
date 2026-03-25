#include "TextPass.hpp"

#include <stdexcept>

namespace Qadra::Render
{
  void TextPass::init ( const QString &vertexSource, const QString &fragmentSource )
  {
    GL::Shader vertexShader ( GL::Shader::Type::Vertex );
    GL::Shader fragmentShader ( GL::Shader::Type::Fragment );

    if ( ! vertexShader.compile ( vertexSource ) || ! fragmentShader.compile ( fragmentSource ) )
      throw std::runtime_error ( "TextPass: shader compilation failed" );

    if ( ! m_program.link ( vertexShader, fragmentShader ) )
      throw std::runtime_error ( "TextPass: program linking failed" );

    m_vao.attribute ( { .index = 0,
                        .size = 2,
                        .type = GL_FLOAT,
                        .relativeOffset = offsetof ( Vertex, position ) } );
    m_vao.attribute (
        { .index = 1, .size = 2, .type = GL_FLOAT, .relativeOffset = offsetof ( Vertex, uv ) } );
    m_vao.attribute (
        { .index = 2, .size = 4, .type = GL_FLOAT, .relativeOffset = offsetof ( Vertex, color ) } );
    m_vao.attribute (
        { .index = 3, .size = 1, .type = GL_FLOAT, .relativeOffset = offsetof ( Vertex, depth ) } );
  }

  void TextPass::upload ( const std::span<const Vertex> vertices )
  {
    if ( vertices.empty () )
    {
      m_vertexCount = 0;
      return;
    }

    m_vbo.allocate ( vertices, GL::Buffer::Usage::StaticDraw );
    m_vao.attachVertexBuffer ( 0, m_vbo, 0, sizeof ( Vertex ) );
    m_vertexCount = vertices.size ();
  }

  void TextPass::render ( const Core::Camera &camera, const GL::Texture &atlas,
                          const double distanceFieldRange ) const
  {
    if ( m_vertexCount == 0 ) return;

    m_vao.bind ();
    m_program.bind ();
    m_program.uniform ( "u_viewProjection", camera.viewProjection () );
    m_program.uniform ( "u_distanceFieldRange", static_cast<float> ( distanceFieldRange ) );

    atlas.bind ( 0 );
    m_program.uniform ( "u_texture", 0 );

    // Keep depth testing against previously rendered geometry, but do not let
    // alpha-blended glyph edges punch holes into later overlapping glyphs.
    GLboolean depthMask = GL_TRUE;
    glGetBooleanv ( GL_DEPTH_WRITEMASK, &depthMask );
    glDepthMask ( GL_FALSE );
    glDrawArrays ( GL_TRIANGLES, 0, static_cast<GLsizei> ( m_vertexCount ) );
    glDepthMask ( depthMask );
  }
} // namespace Qadra::Render
