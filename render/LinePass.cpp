#include "LinePass.hpp"

#include <stdexcept>

namespace Qadra::Render
{
  LinePass::LinePass () : RenderPass ( "line" ) { }

  void LinePass::render ( const Core::Camera &camera ) const
  {
    if ( ! beginRender ( camera ) ) return;

    glLineWidth ( 1.0f );
    glDrawArrays ( GL_LINES, 0, static_cast<GLsizei> ( m_vertexCount ) );
  }

  void LinePass::setupAttributes ()
  {
    m_vao.attribute ( { .index = 0,
                        .size = 2,
                        .type = GL_DOUBLE,
                        .relativeOffset = offsetof ( Vertex, position ) } );
    m_vao.attribute (
        { .index = 1, .size = 4, .type = GL_FLOAT, .relativeOffset = offsetof ( Vertex, color ) } );
    m_vao.attribute (
        { .index = 2, .size = 1, .type = GL_FLOAT, .relativeOffset = offsetof ( Vertex, depth ) } );
  }
} // namespace Qadra::Render
