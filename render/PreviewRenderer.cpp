#include "PreviewRenderer.hpp"

#include <array>

namespace Qadra::Render
{
  void PreviewRenderer::init () { m_linePass.init (); }

  void PreviewRenderer::sync ( const std::span<const PreviewLine> lines )
  {
    m_vertices.clear ();
    m_vertices.reserve ( lines.size () * 2 );

    for ( const auto &line : lines )
    {
      m_vertices.push_back ( { line.start, line.color, 0 } );
      m_vertices.push_back ( { line.end, line.color, 0 } );
    }

    m_batch.upload ( std::span<const LinePass::Vertex> ( m_vertices ),
                     GL::Buffer::Usage::DynamicDraw );
  }

  void PreviewRenderer::draw ( const Core::Camera &camera ) const
  {
    if ( m_vertices.empty () ) return;

    const std::array<GLint, 1> firsts{ 0 };
    const std::array<GLsizei, 1> counts{ static_cast<GLsizei> ( m_vertices.size () ) };
    m_linePass.renderRanges ( camera, m_batch.buffer (), firsts, counts, 1.0f );
  }
} // namespace Qadra::Render
