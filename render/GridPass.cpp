#include "GridPass.hpp"

#include "math/Grid.hpp"

#include <cstddef>
#include <stdexcept>
#include <vector>

namespace Qadra::Render
{
  GridPass::GridPass () : RenderPass ( "grid" ) { }

  void GridPass::render ( const Core::Camera &camera )
  {
    const auto viewport = glm::dvec2 ( camera.width (), camera.height () );
    std::vector<Segment> segments = buildSegments ( camera );
    if ( segments.empty () ) return;

    upload ( std::span<const Segment> ( segments ), GL::Buffer::Usage::DynamicDraw );

    if ( ! beginRender ( camera ) ) return;

    m_program.uniform ( "u_viewportSizePixels", viewport );

    glDrawArraysInstanced ( GL_TRIANGLES, 0, 6, m_vertexCount );
  }

  void GridPass::setupAttributes ()
  {
    m_vao.attribute ( { .index = 0,
                        .size = 2,
                        .type = GL_DOUBLE,
                        .relativeOffset = offsetof ( Segment, from ) } );
    m_vao.attribute (
        { .index = 1, .size = 2, .type = GL_DOUBLE, .relativeOffset = offsetof ( Segment, to ) } );
    m_vao.attribute ( { .index = 2,
                        .size = 4,
                        .type = GL_FLOAT,
                        .relativeOffset = offsetof ( Segment, color ) } );
    m_vao.attribute ( { .index = 3,
                        .size = 1,
                        .type = GL_FLOAT,
                        .relativeOffset = offsetof ( Segment, lineWidthPixels ) } );
    m_vao.attribute ( { .index = 4,
                        .size = 1,
                        .type = GL_FLOAT,
                        .relativeOffset = offsetof ( Segment, antiAliasWidthPixels ) } );
    m_vao.bindingDivisor ( 0, 1 );
  }

  std::vector<GridPass::Segment> GridPass::buildSegments ( const Core::Camera &camera )
  {
    std::vector<Segment> segments;
    std::vector<Math::Grid::Line> lines = Math::Grid::compute ( camera );

    if ( const auto viewport = glm::dvec2 ( camera.width (), camera.height () );
         lines.empty () || viewport.x <= 0.0f || viewport.y <= 0.0f )
      return segments;

    segments.reserve ( lines.size () );

    constexpr auto minorColor = glm::vec4 ( 0.27f, 0.31f, 0.36f, 0.18f );
    constexpr auto majorColor = glm::vec4 ( 0.38f, 0.43f, 0.50f, 0.30f );
    constexpr auto axisColor = glm::vec4 ( 0.56f, 0.63f, 0.72f, 0.55f );

    for ( const auto &[start, end, major] : lines )
    {
      const bool axis = ( std::abs ( start.x ) <= 1e-12 && std::abs ( end.x ) <= 1e-12 ) ||
                        ( std::abs ( start.y ) <= 1e-12 && std::abs ( end.y ) <= 1e-12 );

      segments.push_back ( {
          .from = start,
          .to = end,
          .color = axis ? axisColor : ( major ? majorColor : minorColor ),
          .lineWidthPixels = axis ? 1.15f : 1.0f,
          .antiAliasWidthPixels = axis ? 1.05f : 0.9f,
      } );
    }
    return segments;
  }
} // namespace Qadra::Render
