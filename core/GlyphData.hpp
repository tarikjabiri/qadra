//
#ifndef QADRA_CORE_GLYPH_DATA_HPP
#define QADRA_CORE_GLYPH_DATA_HPP

#include <glm/glm.hpp>

namespace Qadra::Core
{
  struct GlyphData
  {
    glm::vec2 quadMin{ 0.0f };
    glm::vec2 quadMax{ 0.0f };
    glm::vec2 uvMin{ 0.0f };
    glm::vec2 uvMax{ 0.0f };
    bool empty{ true };
  };
} // namespace Qadra::Core

#endif // QADRA_CORE_GLYPH_DATA_HPP
