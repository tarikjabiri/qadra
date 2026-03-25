#ifndef QADRA_RENDERER_HPP
#define QADRA_RENDERER_HPP

#include "Camera.hpp"
#include "Document.hpp"
#include "GeometryCache.hpp"
#include "GridPass.hpp"

namespace Qadra::Render
{
  class Renderer
  {
  public:
    void init ();
    void render ( const Cad::Document &document, const Core::Camera &camera, Core::Font &font );

  private:
    GeometryCache m_cache;
    GridPass m_grid;
  };
} // namespace Qadra::Render

#endif // QADRA_RENDERER_HPP
