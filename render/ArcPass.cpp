#include "ArcPass.hpp"

namespace
{
  using Layout = Qadra::GL::VertexLayout;
  using Instance = Qadra::Render::ArcPass::Instance;

  constexpr std::array<Qadra::GL::VertexAttribute, 9> kArcInstanceAttributes{
      Layout::attribute ( 0, 2, GL_DOUBLE, offsetof ( Instance, centerWorld ) ),
      Layout::attribute ( 1, 2, GL_FLOAT, offsetof ( Instance, boundsMinLocal ) ),
      Layout::attribute ( 2, 2, GL_FLOAT, offsetof ( Instance, boundsMaxLocal ) ),
      Layout::attribute ( 3, 1, GL_FLOAT, offsetof ( Instance, radius ) ),
      Layout::attribute ( 4, 1, GL_FLOAT, offsetof ( Instance, startAngle ) ),
      Layout::attribute ( 5, 1, GL_FLOAT, offsetof ( Instance, sweepAngle ) ),
      Layout::normalizedAttribute ( 6, 4, GL_UNSIGNED_BYTE, offsetof ( Instance, color ) ),
      Layout::integerAttribute ( 7, 1, GL_UNSIGNED_INT, offsetof ( Instance, renderKey ) ),
      Layout::integerAttribute ( 8, 1, GL_UNSIGNED_INT, offsetof ( Instance, flags ) ),
  };

  constexpr std::array<Qadra::GL::VertexBinding, 1> kArcInstanceBindings{
      Layout::binding ( 0, 1 ),
  };

  constexpr Layout kArcInstanceLayout{ kArcInstanceAttributes, kArcInstanceBindings };
} // namespace

namespace Qadra::Render
{
  ArcPass::ArcPass () : RenderPass ( "arc" ) { }

  void ArcPass::renderRanges ( const Core::Camera &camera, const GL::Buffer &instanceBuffer,
                               const std::span<const GLint> firsts,
                               const std::span<const GLsizei> counts,
                               const float renderKeyScale ) const
  {
    if ( firsts.empty () || firsts.size () != counts.size () ) return;
    if ( ! beginRender ( camera, 1 ) ) return;

    m_vao.attachVertexBuffer<Instance> ( 0, instanceBuffer );
    m_program.uniform ( "u_renderKeyScale", renderKeyScale );
    m_program.uniform ( "u_pixelSizeWorld", static_cast<float> ( camera.pixelSizeInWorld () ) );

    const GLboolean blendEnabled = glIsEnabled ( GL_BLEND );
    const GLboolean sampleAlphaToCoverageEnabled = glIsEnabled ( GL_SAMPLE_ALPHA_TO_COVERAGE );
    GLboolean colorMask[4]{ GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE };
    glGetBooleanv ( GL_COLOR_WRITEMASK, colorMask );

    glDisable ( GL_BLEND );
    glEnable ( GL_SAMPLE_ALPHA_TO_COVERAGE );
    glColorMask ( colorMask[0], colorMask[1], colorMask[2], GL_FALSE );

    for ( std::size_t i = 0; i < firsts.size (); i++ )
    {
      if ( counts[i] <= 0 ) continue;
      glDrawArraysInstancedBaseInstance ( GL_TRIANGLES, 0, 6, counts[i],
                                          static_cast<GLuint> ( firsts[i] ) );
    }

    if ( ! sampleAlphaToCoverageEnabled ) glDisable ( GL_SAMPLE_ALPHA_TO_COVERAGE );
    glColorMask ( colorMask[0], colorMask[1], colorMask[2], colorMask[3] );
    if ( blendEnabled ) glEnable ( GL_BLEND );
  }

  void ArcPass::setupAttributes () { m_vao.applyLayout ( kArcInstanceLayout ); }
} // namespace Qadra::Render
