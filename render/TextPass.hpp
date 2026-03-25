#ifndef QADRA_TEXT_PASS_HPP
#define QADRA_TEXT_PASS_HPP

#include "Camera.hpp"
#include "RenderPass.hpp"
#include "gl/Texture.hpp"

#include <glm/glm.hpp>

namespace Qadra::Render
{
  class TextPass : public RenderPass
  {
  public:
    struct Vertex
    {
      glm::vec2 position;
      glm::vec2 uv;
      glm::vec4 color;
      float depth;
    };

    TextPass ();

    void render ( const Core::Camera &camera, const GL::Texture &atlas,
                  double distanceFieldRange ) const;

  protected:
    void setupAttributes () override;
  };
} // namespace Qadra::Render

#endif // QADRA_TEXT_PASS_HPP
