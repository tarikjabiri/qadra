#ifndef QADRA_SHAPED_GLYPH_HPP
#define QADRA_SHAPED_GLYPH_HPP

#include <cstdint>
#include <glm/glm.hpp>

namespace Qadra::Core
{
  struct ShapedGlyph
  {
    std::uint32_t glyphId{};
    std::uint32_t cluster{};
    glm::dvec2 offset{ 0.0 };
    glm::dvec2 advance{ 0.0 };
  };
} // namespace Qadra::Core

#endif // QADRA_SHAPED_GLYPH_HPP
