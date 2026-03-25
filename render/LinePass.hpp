#ifndef QADRA_LINE_PASS_HPP
#define QADRA_LINE_PASS_HPP

#include "Camera.hpp"
#include "RenderPass.hpp"

#include <glm/glm.hpp>

namespace Qadra::Render
{
  class LinePass : public RenderPass
  {
  public:
    struct Vertex
    {
      glm::dvec2 position;
      glm::vec4 color;
      float depth;
    };

    LinePass ();

    void render ( const Core::Camera &camera ) const;

  protected:
    void setupAttributes () override;
  };
} // namespace Qadra::Render

#endif // QADRA_LINE_PASS_HPP
