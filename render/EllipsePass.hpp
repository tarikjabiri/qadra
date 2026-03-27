#ifndef QADRA_RENDER_ELLIPSE_PASS_HPP
#define QADRA_RENDER_ELLIPSE_PASS_HPP

#include "Camera.hpp"
#include "RenderPass.hpp"
#include "gl/Buffer.hpp"

#include <array>
#include <cstdint>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <span>

namespace Qadra::Render
{
  class EllipsePass : public RenderPass
  {
  public:
    struct Instance
    {
      glm::dvec2 centerWorld{ 0.0 };
      glm::vec2 boundsMinLocal{ 0.0f };
      glm::vec2 boundsMaxLocal{ 0.0f };
      glm::vec2 majorDirection{ 1.0f, 0.0f };
      float majorRadius = 0.0f;
      float minorRadius = 0.0f;
      std::array<std::uint8_t, 4> color{ 255, 255, 255, 255 };
      std::uint32_t renderKey = 0;
    };

    EllipsePass ();

    void renderRanges ( const Core::Camera &camera, const GL::Buffer &instanceBuffer,
                        std::span<const GLint> firsts, std::span<const GLsizei> counts,
                        float renderKeyScale ) const;

  protected:
    void setupAttributes () override;
  };
} // namespace Qadra::Render

#endif // QADRA_RENDER_ELLIPSE_PASS_HPP
