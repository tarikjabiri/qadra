#include "Renderer.hpp"

namespace Qadra::Render
{
  void Renderer::init ()
  {
    m_grid.init ();
    m_scene.init ();
  }

  void Renderer::render ( const Cad::Document &document, const Core::Camera &camera,
                          Core::Font &font )
  {
    glEnable ( GL_BLEND );
    glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    // glEnable ( GL_MULTISAMPLE );

    glViewport ( 0, 0, camera.width (), camera.height () );
    glClearColor ( 0.09f, 0.10f, 0.12f, 1.0f );
    glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    m_grid.render ( camera );

    m_scene.sync ( document, font );

    glEnable ( GL_DEPTH_TEST );
    m_scene.draw ( camera, font );
    glDisable ( GL_DEPTH_TEST );
  }
} // namespace Qadra::Render
