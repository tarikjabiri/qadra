#include "RenderPass.hpp"

#include <QFile>
#include <QTextStream>
#include <qcoreapplication.h>
#include <utility>

namespace Qadra::Render
{
  RenderPass::RenderPass ( QString shaderName ) : m_shaderName ( std::move ( shaderName ) ) { }

  void RenderPass::init ()
  {
    const QString shaderDir = QCoreApplication::applicationDirPath () + "/shaders";
    const QString vertexPath = shaderDir + "/" + m_shaderName + ".vertex.glsl";
    const QString fragmentPath = shaderDir + "/" + m_shaderName + ".fragment.glsl";

    const QString vertexSource = loadFileContent ( vertexPath );
    const QString fragmentSource = loadFileContent ( fragmentPath );

    GL::Shader vertexShader ( GL::Shader::Type::Vertex );
    GL::Shader fragmentShader ( GL::Shader::Type::Fragment );

    if ( ! vertexShader.compile ( vertexSource ) || ! fragmentShader.compile ( fragmentSource ) )
      throw std::runtime_error ( "RenderPass [" + m_shaderName.toStdString () +
                                 "]: shader compilation failed" );

    if ( ! m_program.link ( vertexShader, fragmentShader ) )
      throw std::runtime_error ( "RenderPass [" + m_shaderName.toStdString () +
                                 "]: program linking failed" );

    setupAttributes ();
  }

  void RenderPass::bind () const
  {
    m_vao.bind ();
    m_program.bind ();
  }

  bool RenderPass::beginRender ( const Core::Camera &camera ) const
  {
    return beginRender ( camera, m_vertexCount );
  }

  bool RenderPass::beginRender ( const Core::Camera &camera, const std::size_t vertexCount ) const
  {
    if ( vertexCount == 0 ) return false;

    bind ();
    m_program.uniform ( "u_viewProjection", camera.viewProjection () );
    return true;
  }

  QString RenderPass::loadFileContent ( const QString &path )
  {
    QFile file ( path );
    if ( ! file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
      throw std::runtime_error ( "Failed to open shader: " + path.toStdString () );
    return QTextStream ( &file ).readAll ();
  }
} // namespace Qadra::Render
