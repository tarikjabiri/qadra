#ifndef QADRA_LINE_PASS_HPP
#define QADRA_LINE_PASS_HPP

#include "Camera.hpp"
#include "RenderPass.hpp"
#include "gl/Buffer.hpp"

#include <cstdint>
#include <glm/glm.hpp>
#include <span>

namespace Qadra::Render
{
  class LinePass : public RenderPass
  {
  public:
    struct Vertex
    {
      glm::dvec2 position;
      glm::vec4 color;
      std::uint32_t renderKey;
      std::uint32_t flags = 0;
    };

    LinePass ();

    void renderRanges ( const Core::Camera &camera, const GL::Buffer &buffer,
                        std::span<const GLint> firsts, std::span<const GLsizei> counts,
                        float renderKeyScale ) const;

  protected:
    void setupAttributes () override;
  };
} // namespace Qadra::Render

#endif // QADRA_LINE_PASS_HPP
