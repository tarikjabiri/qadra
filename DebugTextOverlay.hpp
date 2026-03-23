#ifndef QADRA_UI_DEBUGTEXTOVERLAY_HPP
#define QADRA_UI_DEBUGTEXTOVERLAY_HPP

#include <span>
#include <string>
#include <vector>

#include "TextEntity.hpp"

namespace Qadra::Core {
  class Font;
}

namespace Qadra::Ui {
  class DebugTextOverlay {
  public:
    struct FontEntry {
      std::string key;
      Qadra::Core::Font *font{};
    };

    void rebuild(std::span<const FontEntry> fonts);

    void clear();

    [[nodiscard]] const std::vector<Qadra::Core::TextEntityData> &textEntities() const noexcept {
      return m_textEntities;
    }

  private:
    std::vector<Qadra::Core::TextEntityData> m_textEntities;
  };
} // Qadra::Ui

#endif // QADRA_UI_DEBUGTEXTOVERLAY_HPP
