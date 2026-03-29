#include "TextPass.hpp"

#include <stdexcept>

namespace Qadra::Render
{
  TextPass::TextPass () : RenderPass ( "text" ) { }

  void TextPass::renderIndirect ( const Core::Camera &camera, const GL::Texture &atlas,
                                  const double distanceFieldRange, const GL::Buffer &instanceBuffer,
                                  const GL::Buffer &commandBuffer, const GLsizei drawCount,
                                  const float renderKeyScale ) const
  {
    if ( drawCount <= 0 ) return;
    if ( ! beginRender ( camera, 1 ) ) return;

    m_vao.attachVertexBuffer ( 0, instanceBuffer, 0, sizeof ( Instance ) );

    m_program.uniform ( "u_distanceFieldRange", static_cast<float> ( distanceFieldRange ) );
    m_program.uniform ( "u_renderKeyScale", renderKeyScale );
    atlas.bind ( 0 );
    m_program.uniform ( "u_texture", 0 );

    GLboolean depthMask = GL_TRUE;
    glGetBooleanv ( GL_DEPTH_WRITEMASK, &depthMask );
    glDepthMask ( GL_FALSE );
    commandBuffer.bind ();
    glMultiDrawArraysIndirect ( GL_TRIANGLES, nullptr, drawCount, 0 );
    commandBuffer.unbind ();
    glDepthMask ( depthMask );
  }

  void TextPass::setupAttributes ()
  {
    m_vao.attribute ( { .index = 0,
                        .size = 2,
                        .type = GL_FLOAT,
                        .relativeOffset = offsetof ( Instance, textOriginWorld ) } );
    m_vao.attribute ( { .index = 1,
                        .size = 2,
                        .type = GL_FLOAT,
                        .relativeOffset = offsetof ( Instance, quadMinLocal ) } );
    m_vao.attribute ( { .index = 2,
                        .size = 2,
                        .type = GL_FLOAT,
                        .relativeOffset = offsetof ( Instance, quadMaxLocal ) } );
    m_vao.attribute ( { .index = 3,
                        .size = 2,
                        .type = GL_UNSIGNED_SHORT,
                        .relativeOffset = offsetof ( Instance, uvMin ),
                        .normalized = GL_TRUE } );
    m_vao.attribute ( { .index = 4,
                        .size = 2,
                        .type = GL_UNSIGNED_SHORT,
                        .relativeOffset = offsetof ( Instance, uvMax ),
                        .normalized = GL_TRUE } );
    m_vao.attribute ( { .index = 5,
                        .size = 2,
                        .type = GL_SHORT,
                        .relativeOffset = offsetof ( Instance, rotation ),
                        .normalized = GL_TRUE } );
    m_vao.attribute ( { .index = 6,
                        .size = 4,
                        .type = GL_UNSIGNED_BYTE,
                        .relativeOffset = offsetof ( Instance, color ),
                        .normalized = GL_TRUE } );
    m_vao.attribute ( { .index = 7,
                        .size = 1,
                        .type = GL_UNSIGNED_INT,
                        .relativeOffset = offsetof ( Instance, renderKey ),
                        .integer = true } );
    m_vao.attribute ( { .index = 8,
                        .size = 1,
                        .type = GL_UNSIGNED_INT,
                        .relativeOffset = offsetof ( Instance, flags ),
                        .integer = true } );
    m_vao.bindingDivisor ( 0, 1 );
  }
} // namespace Qadra::Render
