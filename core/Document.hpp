#ifndef QADRA_CORE_DOCUMENT_HPP
#define QADRA_CORE_DOCUMENT_HPP

#include <cstddef>
#include <unordered_map>
#include <vector>

#include "TextEntity.hpp"

namespace Qadra::Core {
  class Document {
  public:
    [[nodiscard]] EntityId createText(TextEntityData textEntity);

    [[nodiscard]] bool updateText(EntityId id, TextEntityData textEntity);

    [[nodiscard]] bool remove(EntityId id);

    void clear();

    [[nodiscard]] const TextEntityData *text(EntityId id) const;

    [[nodiscard]] TextEntityData *text(EntityId id);

    [[nodiscard]] const std::vector<TextEntityData> &entities() const noexcept { return m_entities; }

  private:
    [[nodiscard]] std::size_t *index(EntityId id);

    [[nodiscard]] const std::size_t *index(EntityId id) const;

    EntityId m_nextEntityId{1};
    std::vector<TextEntityData> m_entities;
    std::unordered_map<EntityId, std::size_t> m_entityIndices;
  };
} // Qadra::Core

#endif // QADRA_CORE_DOCUMENT_HPP
