#ifndef QADRA_TEXT_PASS_HPP
#define QADRA_TEXT_PASS_HPP

#include "Buffer.hpp"
#include "Camera.hpp"
#include "Program.hpp"
#include "Texture.hpp"
#include "VertexArray.hpp"

#include <glm/glm.hpp>
#include <span>

namespace Qadra::Render
{
  class TextPass
  {
  public:
    struct Vertex
    {
      glm::vec2 position;
      glm::vec2 uv;
      glm::vec4 color;
      float depth;
    };

    TextPass () = default;

    void init ( const QString &vertexSource, const QString &fragmentSource );

    void upload ( std::span<const Vertex> vertices );

    void render ( const Core::Camera &camera, const GL::Texture &atlas,
                  double distanceFieldRange ) const;

  private:
    GL::Buffer m_vbo;
    GL::VertexArray m_vao;
    GL::Program m_program;
    std::size_t m_vertexCount = 0;
  };
} // namespace Qadra::Render

#endif // QADRA_TEXT_PASS_HPP
