#ifndef QADRA_RENDERER_HPP
#define QADRA_RENDERER_HPP

#include "Camera.hpp"
#include "Document.hpp"
#include "GridPass.hpp"
#include "PreviewRenderer.hpp"
#include "RenderScene.hpp"

#include <span>

namespace Qadra::Render
{
  class Renderer
  {
  public:
    void init ();
    void render ( const Cad::Document &document, const Core::Camera &camera, Core::Font &font,
                  const PreviewScene &preview );

  private:
    GridPass m_grid;
    RenderScene m_scene;
    PreviewRenderer m_preview;
  };
} // namespace Qadra::Render

#endif // QADRA_RENDERER_HPP
