#include "TextEntity.hpp"

namespace Qadra::Core {
  TextEntityData buildTextEntityData(const EntityId id,
                                     const TextEntityBuildInput &input,
                                     Font &font) {
    const TextLayoutBounds localBounds = measureTextLocalBounds(font, input.content, input.heightWorld);
    const std::array<glm::dvec2, 4> orientedCorners = orientedTextCorners(
      input.position,
      localBounds,
      input.rotationRadians
    );
    const TextLayoutBounds worldBounds = axisAlignedBounds(orientedCorners);

    return {
      .id = id,
      .position = input.position,
      .heightWorld = input.heightWorld,
      .rotationRadians = input.rotationRadians,
      .content = input.content,
      .fontKey = input.fontKey,
      .color = input.color,
      .localBounds = localBounds,
      .worldBounds = worldBounds,
    };
  }
} // Qadra::Core
