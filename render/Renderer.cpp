#include "Renderer.hpp"

namespace Qadra::Render
{
  void Renderer::init ( const QString &shaderDir ) { m_cache.init ( shaderDir ); }

  void Renderer::render ( const Cad::Document &document, const Core::Camera &camera,
                          Core::Font &font )
  {
    m_cache.sync ( document, font );
    m_cache.draw ( camera, font );
  }
} // namespace Qadra::Render
