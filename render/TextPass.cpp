#include "TextPass.hpp"

#include <stdexcept>

namespace Qadra::Render
{
  TextPass::TextPass () : RenderPass ( "text" ) { }

  void TextPass::render ( const Core::Camera &camera, const GL::Texture &atlas,
                          const double distanceFieldRange ) const
  {
    if ( ! beginRender ( camera ) ) return;

    m_program.uniform ( "u_distanceFieldRange", static_cast<float> ( distanceFieldRange ) );
    atlas.bind ( 0 );
    m_program.uniform ( "u_texture", 0 );

    GLboolean depthMask = GL_TRUE;
    glGetBooleanv ( GL_DEPTH_WRITEMASK, &depthMask );
    glDepthMask ( GL_FALSE );
    glDrawArrays ( GL_TRIANGLES, 0, static_cast<GLsizei> ( m_vertexCount ) );
    glDepthMask ( depthMask );
  }

  void TextPass::setupAttributes ()
  {
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
} // namespace Qadra::Render
