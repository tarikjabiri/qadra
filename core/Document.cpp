#include "Document.hpp"

#include <utility>

namespace Qadra::Core {
  EntityId Document::createText(TextEntityData textEntity) {
    const EntityId id = m_nextEntityId++;
    textEntity.id = id;

    m_entityIndices[id] = m_entities.size();
    m_entities.push_back(std::move(textEntity));
    return id;
  }

  bool Document::updateText(const EntityId id, TextEntityData textEntity) {
    TextEntityData *existingEntity = text(id);
    if (!existingEntity) {
      return false;
    }

    textEntity.id = id;
    *existingEntity = std::move(textEntity);
    return true;
  }

  bool Document::remove(const EntityId id) {
    const std::size_t *entityIndex = index(id);
    if (!entityIndex) {
      return false;
    }

    m_entities.erase(m_entities.begin() + static_cast<std::ptrdiff_t>(*entityIndex));
    m_entityIndices.erase(id);

    for (std::size_t currentIndex = *entityIndex; currentIndex < m_entities.size(); ++currentIndex) {
      m_entityIndices[m_entities[currentIndex].id] = currentIndex;
    }

    return true;
  }

  void Document::clear() {
    m_entities.clear();
    m_entityIndices.clear();
  }

  const TextEntityData *Document::text(const EntityId id) const {
    const std::size_t *entityIndex = index(id);
    return entityIndex ? &m_entities[*entityIndex] : nullptr;
  }

  TextEntityData *Document::text(const EntityId id) {
    return const_cast<TextEntityData *>(std::as_const(*this).text(id));
  }

  std::size_t *Document::index(const EntityId id) {
    return const_cast<std::size_t *>(std::as_const(*this).index(id));
  }

  const std::size_t *Document::index(const EntityId id) const {
    const auto iterator = m_entityIndices.find(id);
    return iterator != m_entityIndices.end() ? &iterator->second : nullptr;
  }
} // Qadra::Core
