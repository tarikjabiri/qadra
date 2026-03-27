#include "PreviewRenderer.hpp"

#include "ArcInstanceBuilder.hpp"
#include "EllipseInstanceBuilder.hpp"

#include <array>

namespace Qadra::Render
{
  void PreviewRenderer::init ()
  {
    m_linePass.init ();
    m_arcPass.init ();
    m_ellipsePass.init ();
  }

  void PreviewRenderer::sync ( const PreviewScene &preview )
  {
    m_vertices.clear ();
    m_vertices.reserve ( preview.lines.size () * 2 );

    for ( const auto &line : preview.lines )
    {
      m_vertices.push_back ( { line.start, line.color, 0 } );
      m_vertices.push_back ( { line.end, line.color, 0 } );
    }

    m_batch.upload ( std::span<const LinePass::Vertex> ( m_vertices ),
                     GL::Buffer::Usage::DynamicDraw );

    m_arcInstances.clear ();
    m_arcInstances.reserve ( preview.arcs.size () );
    for ( const auto &arc : preview.arcs )
    {
      m_arcInstances.push_back ( buildArcInstance (
          Math::Arc ( arc.center, arc.radius, arc.startAngle, arc.sweepAngle ), arc.color, 0 ) );
    }

    m_arcBatch.upload ( std::span<const ArcPass::Instance> ( m_arcInstances ),
                        GL::Buffer::Usage::DynamicDraw );

    m_ellipseInstances.clear ();
    m_ellipseInstances.reserve ( preview.ellipses.size () );
    for ( const auto &ellipse : preview.ellipses )
    {
      m_ellipseInstances.push_back (
          buildEllipseInstance ( Math::Ellipse ( ellipse.center, ellipse.majorDirection,
                                                 ellipse.majorRadius, ellipse.minorRadius ),
                                 ellipse.color, 0 ) );
    }

    m_ellipseBatch.upload ( std::span<const EllipsePass::Instance> ( m_ellipseInstances ),
                            GL::Buffer::Usage::DynamicDraw );
  }

  void PreviewRenderer::draw ( const Core::Camera &camera ) const
  {
    if ( ! m_vertices.empty () )
    {
      const std::array<GLint, 1> firsts{ 0 };
      const std::array<GLsizei, 1> counts{ static_cast<GLsizei> ( m_vertices.size () ) };
      m_linePass.renderRanges ( camera, m_batch.buffer (), firsts, counts, 1.0f );
    }

    if ( ! m_arcInstances.empty () )
    {
      const std::array<GLint, 1> firsts{ 0 };
      const std::array<GLsizei, 1> counts{ static_cast<GLsizei> ( m_arcInstances.size () ) };
      m_arcPass.renderRanges ( camera, m_arcBatch.buffer (), firsts, counts, 1.0f );
    }

    if ( ! m_ellipseInstances.empty () )
    {
      const std::array<GLint, 1> firsts{ 0 };
      const std::array<GLsizei, 1> counts{ static_cast<GLsizei> ( m_ellipseInstances.size () ) };
      m_ellipsePass.renderRanges ( camera, m_ellipseBatch.buffer (), firsts, counts, 1.0f );
    }
  }
} // namespace Qadra::Render
