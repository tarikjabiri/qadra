#include "PreviewRenderer.hpp"

#include "ArcInstanceBuilder.hpp"
#include "EllipseInstanceBuilder.hpp"

#include <array>
#include <cmath>

namespace
{
  constexpr double kDashPixels = 14.0;
  constexpr double kGapPixels = 9.0;
  constexpr double kLineEpsilon = 1e-9;

  void appendSolidLine ( std::vector<Qadra::Render::LinePass::Vertex> &vertices,
                         const Qadra::Render::PreviewLine &line )
  {
    vertices.push_back ( { line.start, line.color, 0, 1u } );
    vertices.push_back ( { line.end, line.color, 0, 1u } );
  }

  void appendDashedLine ( std::vector<Qadra::Render::LinePass::Vertex> &vertices,
                          const Qadra::Render::PreviewLine &line, const double pixelSizeWorld )
  {
    const glm::dvec2 delta = line.end - line.start;
    const double length = glm::length ( delta );
    if ( length <= kLineEpsilon ) return;

    const glm::dvec2 direction = delta / length;
    const double dashWorld = kDashPixels * pixelSizeWorld;
    const double gapWorld = kGapPixels * pixelSizeWorld;
    const double stepWorld = dashWorld + gapWorld;

    if ( dashWorld <= kLineEpsilon || stepWorld <= kLineEpsilon )
    {
      appendSolidLine ( vertices, line );
      return;
    }

    for ( double offset = 0.0; offset < length; offset += stepWorld )
    {
      const double dashStart = offset;
      const double dashEnd = std::min ( offset + dashWorld, length );
      vertices.push_back ( { line.start + direction * dashStart, line.color, 0, 1u } );
      vertices.push_back ( { line.start + direction * dashEnd, line.color, 0, 1u } );
    }
  }
} // namespace

namespace Qadra::Render
{
  void PreviewRenderer::init ()
  {
    m_linePass.init ();
    m_arcPass.init ();
    m_ellipsePass.init ();
  }

  void PreviewRenderer::sync ( const PreviewScene &preview, const Core::Camera &camera )
  {
    m_vertices.clear ();
    m_vertices.reserve ( preview.lines.size () * 8 );

    for ( const auto &line : preview.lines )
    {
      switch ( line.style )
      {
        case PreviewLineStyle::Solid:
          appendSolidLine ( m_vertices, line );
          break;
        case PreviewLineStyle::Dashed:
          appendDashedLine ( m_vertices, line, camera.pixelSizeInWorld () );
          break;
      }
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
