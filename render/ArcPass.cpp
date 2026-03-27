#include "ArcPass.hpp"

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

    m_vao.attachVertexBuffer ( 0, instanceBuffer, 0, sizeof ( Instance ) );
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

  void ArcPass::setupAttributes ()
  {
    m_vao.attribute ( { .index = 0,
                        .size = 2,
                        .type = GL_DOUBLE,
                        .relativeOffset = offsetof ( Instance, centerWorld ) } );
    m_vao.attribute ( { .index = 1,
                        .size = 2,
                        .type = GL_FLOAT,
                        .relativeOffset = offsetof ( Instance, boundsMinLocal ) } );
    m_vao.attribute ( { .index = 2,
                        .size = 2,
                        .type = GL_FLOAT,
                        .relativeOffset = offsetof ( Instance, boundsMaxLocal ) } );
    m_vao.attribute ( { .index = 3,
                        .size = 1,
                        .type = GL_FLOAT,
                        .relativeOffset = offsetof ( Instance, radius ) } );
    m_vao.attribute ( { .index = 4,
                        .size = 1,
                        .type = GL_FLOAT,
                        .relativeOffset = offsetof ( Instance, startAngle ) } );
    m_vao.attribute ( { .index = 5,
                        .size = 1,
                        .type = GL_FLOAT,
                        .relativeOffset = offsetof ( Instance, sweepAngle ) } );
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
    m_vao.bindingDivisor ( 0, 1 );
  }
} // namespace Qadra::Render
