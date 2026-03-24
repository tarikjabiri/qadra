#include "Renderer.hpp"

namespace Qadra::Render
{
  void Renderer::init ( const QString &shaderDir ) { m_cache.init ( shaderDir ); }

  void Renderer::render ( const Cad::Document &document, const Core::Camera &camera )
  {
    m_cache.sync ( document );
    m_cache.draw ( camera );
  }
} // namespace Qadra::Render
