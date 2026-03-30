#include "TextPass.hpp"

#include <stdexcept>

namespace
{
  using Layout = Qadra::GL::VertexLayout;
  using Instance = Qadra::Render::TextPass::Instance;

  constexpr std::array<Qadra::GL::VertexAttribute, 11> kTextInstanceAttributes{
      Layout::attribute ( 0, 2, GL_FLOAT, offsetof ( Instance, textOriginWorld ) ),
      Layout::attribute ( 1, 2, GL_FLOAT, offsetof ( Instance, quadMinLocal ) ),
      Layout::attribute ( 2, 2, GL_FLOAT, offsetof ( Instance, quadMaxLocal ) ),
      Layout::attribute ( 3, 2, GL_FLOAT, offsetof ( Instance, textBoxMinLocal ) ),
      Layout::attribute ( 4, 2, GL_FLOAT, offsetof ( Instance, textBoxMaxLocal ) ),
      Layout::normalizedAttribute ( 5, 2, GL_UNSIGNED_SHORT, offsetof ( Instance, uvMin ) ),
      Layout::normalizedAttribute ( 6, 2, GL_UNSIGNED_SHORT, offsetof ( Instance, uvMax ) ),
      Layout::normalizedAttribute ( 7, 2, GL_SHORT, offsetof ( Instance, rotation ) ),
      Layout::normalizedAttribute ( 8, 4, GL_UNSIGNED_BYTE, offsetof ( Instance, color ) ),
      Layout::integerAttribute ( 9, 1, GL_UNSIGNED_INT, offsetof ( Instance, renderKey ) ),
      Layout::integerAttribute ( 10, 1, GL_UNSIGNED_INT, offsetof ( Instance, flags ) ),
  };

  constexpr std::array<Qadra::GL::VertexBinding, 1> kTextInstanceBindings{
      Layout::binding ( 0, 1 ),
  };

  constexpr Layout kTextInstanceLayout{ kTextInstanceAttributes, kTextInstanceBindings };
} // namespace

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

    m_vao.attachVertexBuffer<Instance> ( 0, instanceBuffer );

    m_program.uniform ( "u_distanceFieldRange", static_cast<float> ( distanceFieldRange ) );
    m_program.uniform ( "u_renderKeyScale", renderKeyScale );
    m_program.uniform ( "u_pixelSizeWorld", static_cast<float> ( camera.pixelSizeInWorld () ) );
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

  void TextPass::setupAttributes () { m_vao.applyLayout ( kTextInstanceLayout ); }
} // namespace Qadra::Render
