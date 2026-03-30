#ifndef QADRA_TEXT_PASS_HPP
#define QADRA_TEXT_PASS_HPP

#include "Camera.hpp"
#include "RenderPass.hpp"
#include "gl/Buffer.hpp"
#include "gl/Texture.hpp"

#include <array>
#include <cstdint>
#include <glm/glm.hpp>
#include <span>

namespace Qadra::Render
{
  class TextPass : public RenderPass
  {
  public:
    static constexpr std::uint32_t kFlagAlive = 1u << 0u;
    static constexpr std::uint32_t kFlagMarkerAnchor = 1u << 1u;

    struct Instance
    {
      glm::vec2 textOriginWorld;
      glm::vec2 quadMinLocal;
      glm::vec2 quadMaxLocal;
      glm::vec2 textBoxMinLocal;
      glm::vec2 textBoxMaxLocal;
      std::array<std::uint16_t, 2> uvMin;
      std::array<std::uint16_t, 2> uvMax;
      std::array<std::int16_t, 2> rotation;
      std::array<std::uint8_t, 4> color;
      std::uint32_t renderKey;
      std::uint32_t flags = 0;
    };

    struct DrawCommand
    {
      std::uint32_t count = 6;
      std::uint32_t instanceCount = 0;
      std::uint32_t first = 0;
      std::uint32_t baseInstance = 0;
    };

    TextPass ();

    void renderIndirect ( const Core::Camera &camera, const GL::Texture &atlas,
                          double distanceFieldRange, const GL::Buffer &instanceBuffer,
                          const GL::Buffer &commandBuffer, GLsizei drawCount,
                          float renderKeyScale ) const;

  protected:
    void setupAttributes () override;
  };
} // namespace Qadra::Render

#endif // QADRA_TEXT_PASS_HPP
