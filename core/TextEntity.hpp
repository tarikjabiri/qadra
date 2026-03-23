#ifndef QADRA_CORE_TEXTENTITY_HPP
#define QADRA_CORE_TEXTENTITY_HPP

#include <cstdint>
#include <string>

#include <glm/glm.hpp>

#include "Font.hpp"
#include "TextLayout.hpp"

namespace Qadra::Core {
  using EntityId = std::uint64_t;

  struct TextEntityBuildInput {
    glm::dvec2 position{0.0};
    double heightWorld{};
    double rotationRadians{};
    std::string content;
    std::string fontKey;
    glm::vec4 color{1.0f};
  };

  struct TextEntityData {
    EntityId id{};
    glm::dvec2 position{0.0};
    double heightWorld{};
    double rotationRadians{};
    std::string content;
    std::string fontKey;
    glm::vec4 color{1.0f};
    TextLayoutBounds localBounds;
    TextLayoutBounds worldBounds;
  };

  [[nodiscard]] TextEntityData buildTextEntityData(EntityId id,
                                                   const TextEntityBuildInput &input,
                                                   Font &font);
} // Qadra::Core

#endif // QADRA_CORE_TEXTENTITY_HPP
