#include "LinePass.hpp"

#include <array>
#include <stdexcept>

namespace
{
  using Layout = Qadra::GL::VertexLayout;
  using Vertex = Qadra::Render::LinePass::Vertex;

  constexpr std::array<Qadra::GL::VertexAttribute, 4> kLineVertexAttributes{
      Layout::attribute ( 0, 2, GL_DOUBLE, offsetof ( Vertex, position ) ),
      Layout::attribute ( 1, 4, GL_FLOAT, offsetof ( Vertex, color ) ),
      Layout::integerAttribute ( 2, 1, GL_UNSIGNED_INT, offsetof ( Vertex, renderKey ) ),
      Layout::integerAttribute ( 3, 1, GL_UNSIGNED_INT, offsetof ( Vertex, flags ) ),
  };

  constexpr Layout kLineVertexLayout{ kLineVertexAttributes };
} // namespace

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

    m_vao.attachVertexBuffer<Vertex> ( 0, buffer );
    m_program.uniform ( "u_renderKeyScale", renderKeyScale );

    glLineWidth ( 1.0f );
    glMultiDrawArrays ( GL_LINES, firsts.data (), counts.data (),
                        static_cast<GLsizei> ( firsts.size () ) );
  }

  void LinePass::setupAttributes () { m_vao.applyLayout ( kLineVertexLayout ); }
} // namespace Qadra::Render
