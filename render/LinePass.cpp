#include "LinePass.hpp"

#include <stdexcept>

namespace Qadra::Render
{
  LinePass::LinePass () : RenderPass ( "line" ) { }

  void LinePass::renderRanges ( const Core::Camera &camera, const GL::Buffer &buffer,
                                const std::span<const GLint> firsts,
                                const std::span<const GLsizei> counts,
                                const float renderKeyScale ) const
  {
    if ( firsts.empty () || firsts.size () != counts.size () ) return;
    if ( ! beginRender ( camera, 1 ) ) return;

    m_vao.attachVertexBuffer ( 0, buffer, 0, sizeof ( Vertex ) );
    m_program.uniform ( "u_renderKeyScale", renderKeyScale );

    glLineWidth ( 1.0f );
    glMultiDrawArrays ( GL_LINES, firsts.data (), counts.data (),
                        static_cast<GLsizei> ( firsts.size () ) );
  }

  void LinePass::setupAttributes ()
  {
    m_vao.attribute ( { .index = 0,
                        .size = 2,
                        .type = GL_DOUBLE,
                        .relativeOffset = offsetof ( Vertex, position ) } );
    m_vao.attribute (
        { .index = 1, .size = 4, .type = GL_FLOAT, .relativeOffset = offsetof ( Vertex, color ) } );
    m_vao.attribute ( { .index = 2,
                        .size = 1,
                        .type = GL_UNSIGNED_INT,
                        .relativeOffset = offsetof ( Vertex, renderKey ),
                        .integer = true } );
    m_vao.attribute ( { .index = 3,
                        .size = 1,
                        .type = GL_UNSIGNED_INT,
                        .relativeOffset = offsetof ( Vertex, flags ),
                        .integer = true } );
  }
} // namespace Qadra::Render
