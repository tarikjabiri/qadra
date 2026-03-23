#ifndef QADRA_UI_CANVASFONTCATALOG_HPP
#define QADRA_UI_CANVASFONTCATALOG_HPP

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "Font.hpp"

namespace Qadra::Ui {
  class CanvasFontCatalog {
  public:
    struct LoadedFont {
      std::string key;
      std::unique_ptr<Qadra::Core::Font> font;
    };

    void loadSystemFonts();

    [[nodiscard]] bool empty() const noexcept { return m_loadedFonts.empty(); }

    [[nodiscard]] std::string defaultFontKey() const;

    [[nodiscard]] Qadra::Core::Font *font(std::string_view fontKey);

    [[nodiscard]] const Qadra::Core::Font *font(std::string_view fontKey) const;

    [[nodiscard]] const std::vector<LoadedFont> &loadedFonts() const noexcept { return m_loadedFonts; }

  private:
    std::vector<LoadedFont> m_loadedFonts;
  };
} // Qadra::Ui

#endif // QADRA_UI_CANVASFONTCATALOG_HPP
