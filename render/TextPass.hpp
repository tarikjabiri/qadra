#ifndef QADRA_TEXT_PASS_HPP
#define QADRA_TEXT_PASS_HPP

#include "Camera.hpp"
#include "RenderPass.hpp"
#include "gl/Buffer.hpp"
#include "gl/Texture.hpp"

#include <cstdint>
#include <glm/glm.hpp>
#include <span>

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
      std::uint32_t renderKey;
    };

    TextPass ();

    void renderRanges ( const Core::Camera &camera, const GL::Texture &atlas,
                        double distanceFieldRange, const GL::Buffer &buffer,
                        std::span<const GLint> firsts, std::span<const GLsizei> counts,
                        float renderKeyScale ) const;

  protected:
    void setupAttributes () override;
  };
} // namespace Qadra::Render

#endif // QADRA_TEXT_PASS_HPP
