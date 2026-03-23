#include "CanvasFontCatalog.hpp"

#include <algorithm>
#include <array>
#include <exception>
#include <optional>

#include <QDebug>
#include <QFileInfo>
#include <QString>

namespace {
  struct FontCandidate {
    const char *key;
    std::array<const char *, 3> paths;
  };

  std::optional<QString> firstExistingFontPath(const std::array<const char *, 3> &candidates) {
    for (const char *candidate: candidates) {
      const QString path = QString::fromLatin1(candidate);
      if (QFileInfo::exists(path)) {
        return path;
      }
    }

    return std::nullopt;
  }

  const std::array<FontCandidate, 10> textFontCandidates{{
    {"latinRegular", {"C:/Windows/Fonts/segoeui.ttf", "C:/Windows/Fonts/calibri.ttf", "C:/Windows/Fonts/arial.ttf"}},
    {"latinBold", {"C:/Windows/Fonts/segoeuib.ttf", "C:/Windows/Fonts/calibrib.ttf", "C:/Windows/Fonts/arialbd.ttf"}},
    {"unicodeRegular", {"C:/Windows/Fonts/ARIALUNI.ttf", "C:/Windows/Fonts/arial.ttf", "C:/Windows/Fonts/segoeui.ttf"}},
    {"arabicBold", {"C:/Windows/Fonts/arialbd.ttf", "C:/Windows/Fonts/segoeuib.ttf", "C:/Windows/Fonts/ARIALUNI.ttf"}},
    {"chineseRegular", {"C:/Windows/Fonts/msyh.ttc", "C:/Windows/Fonts/simsun.ttc", "C:/Windows/Fonts/ARIALUNI.ttf"}},
    {"chineseBold", {"C:/Windows/Fonts/msyhbd.ttc", "C:/Windows/Fonts/simsunb.ttf", "C:/Windows/Fonts/ARIALUNI.ttf"}},
    {"japaneseRegular", {"C:/Windows/Fonts/YuGothM.ttc", "C:/Windows/Fonts/msgothic.ttc", "C:/Windows/Fonts/ARIALUNI.ttf"}},
    {"japaneseBold", {"C:/Windows/Fonts/YuGothB.ttc", "C:/Windows/Fonts/msgothic.ttc", "C:/Windows/Fonts/ARIALUNI.ttf"}},
    {"koreanBold", {"C:/Windows/Fonts/malgunbd.ttf", "C:/Windows/Fonts/malgun.ttf", "C:/Windows/Fonts/ARIALUNI.ttf"}},
    {"thaiRegular", {"C:/Windows/Fonts/LeelawUI.ttf", "C:/Windows/Fonts/LeelaUIb.ttf", "C:/Windows/Fonts/ARIALUNI.ttf"}},
  }};
}

namespace Qadra::Ui {
  void CanvasFontCatalog::loadSystemFonts() {
    m_loadedFonts.clear();

    for (const FontCandidate &fontCandidate: textFontCandidates) {
      const std::optional<QString> fontPath = firstExistingFontPath(fontCandidate.paths);
      if (!fontPath) {
        continue;
      }

      try {
        m_loadedFonts.push_back({
          .key = fontCandidate.key,
          .font = std::make_unique<Qadra::Core::Font>(fontPath->toStdString())
        });
      } catch (const std::exception &exception) {
        qWarning() << "Font initialization failed for" << fontCandidate.key << "at" << *fontPath << ":" << exception.what();
      }
    }

    if (m_loadedFonts.empty()) {
      qWarning() << "No text fonts were loaded";
    }
  }

  std::string CanvasFontCatalog::defaultFontKey() const {
    if (font("latinRegular")) {
      return "latinRegular";
    }

    return !m_loadedFonts.empty() ? m_loadedFonts.front().key : std::string{};
  }

  Qadra::Core::Font *CanvasFontCatalog::font(const std::string_view fontKey) {
    const auto iterator = std::find_if(m_loadedFonts.begin(), m_loadedFonts.end(), [&](const LoadedFont &loadedFont) {
      return loadedFont.key == fontKey;
    });

    return iterator != m_loadedFonts.end() ? iterator->font.get() : nullptr;
  }

  const Qadra::Core::Font *CanvasFontCatalog::font(const std::string_view fontKey) const {
    const auto iterator = std::find_if(m_loadedFonts.begin(), m_loadedFonts.end(), [&](const LoadedFont &loadedFont) {
      return loadedFont.key == fontKey;
    });

    return iterator != m_loadedFonts.end() ? iterator->font.get() : nullptr;
  }
} // Qadra::Ui
