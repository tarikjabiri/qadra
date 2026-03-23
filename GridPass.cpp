#include "GridPass.hpp"

#include "math/Grid.hpp"

#include <cstddef>
#include <stdexcept>
#include <vector>

namespace Qadra::Render
{
  void GridPass::init ( const QString &vertexSource, const QString &fragmentSource )
  {
    GL::Shader vertexShader ( GL::Shader::Type::Vertex );
    GL::Shader fragmentShader ( GL::Shader::Type::Fragment );

    if ( ! vertexShader.compile ( vertexSource ) || ! fragmentShader.compile ( fragmentSource ) )
    {
      throw std::runtime_error ( "Vertex shader or fragment shaders are not compiled" );
    }

    if ( ! m_program.link ( vertexShader, fragmentShader ) )
    {
      throw std::runtime_error ( "Program shader or fragment shaders are not linked" );
    }

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

  void GridPass::render ( const Core::Camera &camera, const glm::vec2 &viewportSizePixels ) const
  {
    std::vector<Segment> segments;
    std::vector<Math::Grid::Line> lines = Math::Grid::compute ( camera );

    if ( lines.empty () || viewportSizePixels.x <= 0.0f || viewportSizePixels.y <= 0.0f ) return;

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

    m_vbo.allocate ( std::span<const Segment> ( segments ), GL::Buffer::Usage::DynamicDraw );

    m_vao.attachVertexBuffer ( 0, m_vbo, 0, sizeof ( Segment ) );

    m_vao.bind ();
    m_program.bind ();
    m_program.uniform ( "u_viewProjection", camera.viewProjection () );
    m_program.uniform ( "u_viewportSizePixels", viewportSizePixels );

    glDrawArraysInstanced ( GL_TRIANGLES, 0, 6, static_cast<GLsizei> ( segments.size () ) );
  }
} // namespace Qadra::Render
