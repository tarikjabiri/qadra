#include "TextPass.hpp"

#include <stdexcept>

namespace Qadra::Render
{
  TextPass::TextPass () : RenderPass ( "text" ) { }

  void TextPass::renderRanges ( const Core::Camera &camera, const GL::Texture &atlas,
                                const double distanceFieldRange, const GL::Buffer &buffer,
                                const std::span<const GLint> firsts,
                                const std::span<const GLsizei> counts,
                                const float renderKeyScale ) const
  {
    if ( firsts.empty () || firsts.size () != counts.size () ) return;
    if ( ! beginRender ( camera, 1 ) ) return;

    m_vao.attachVertexBuffer ( 0, buffer, 0, sizeof ( Vertex ) );

    m_program.uniform ( "u_distanceFieldRange", static_cast<float> ( distanceFieldRange ) );
    m_program.uniform ( "u_renderKeyScale", renderKeyScale );
    atlas.bind ( 0 );
    m_program.uniform ( "u_texture", 0 );

    GLboolean depthMask = GL_TRUE;
    glGetBooleanv ( GL_DEPTH_WRITEMASK, &depthMask );
    glDepthMask ( GL_FALSE );
    glMultiDrawArrays ( GL_TRIANGLES, firsts.data (), counts.data (),
                        static_cast<GLsizei> ( firsts.size () ) );
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
    m_vao.attribute ( { .index = 3,
                        .size = 1,
                        .type = GL_UNSIGNED_INT,
                        .relativeOffset = offsetof ( Vertex, renderKey ),
                        .integer = true } );
  }
} // namespace Qadra::Render
