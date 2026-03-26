#ifndef QADRA_RENDERER_HPP
#define QADRA_RENDERER_HPP

#include "Camera.hpp"
#include "Document.hpp"
#include "GridPass.hpp"
#include "RenderScene.hpp"

namespace Qadra::Render
{
  class Renderer
  {
  public:
    void init ();
    void render ( const Cad::Document &document, const Core::Camera &camera, Core::Font &font );

  private:
    GridPass m_grid;
    RenderScene m_scene;
  };
} // namespace Qadra::Render

#endif // QADRA_RENDERER_HPP
