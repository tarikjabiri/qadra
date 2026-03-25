#ifndef QADRA_RENDER_GRIDPASS_HPP
#define QADRA_RENDER_GRIDPASS_HPP

#include "Camera.hpp"
#include "gl/Program.hpp"
#include "RenderPass.hpp"

namespace Qadra::Render
{
  class GridPass : public RenderPass
  {
  public:
    struct Segment
    {
      glm::dvec2 from;
      glm::dvec2 to;
      glm::vec4 color;
      float lineWidthPixels = 1.0f;
      float antiAliasWidthPixels = 1.0f;
    };

    GridPass ();

    void render ( const Core::Camera &camera );

  protected:
    void setupAttributes () override;

  private:
    static std::vector<Segment> buildSegments ( const Core::Camera &camera );
  };
} // namespace Qadra::Render

#endif // QADRA_RENDER_GRIDPASS_HPP
