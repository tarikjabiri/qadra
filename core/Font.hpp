#ifndef QADRA_CORE_FONT_HPP
#define QADRA_CORE_FONT_HPP

#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <glm/glm.hpp>

#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Texture.hpp"

namespace msdfgen {
  class FontHandle;
  class FreetypeHandle;
}

namespace Qadra::Core {
  struct GlyphInfo {
    glm::vec2 planeBoundsMin{0.0f};
    glm::vec2 planeBoundsMax{0.0f};
    glm::vec2 uvMin{0.0f};
    glm::vec2 uvMax{0.0f};
    bool cached{false};
  };

  struct GlyphDebugImage {
    int widthPixels{};
    int heightPixels{};
    std::vector<float> pixels;
  };

  struct ShapedGlyph {
    std::uint32_t glyphId{};
    glm::dvec2 offset{0.0};
    glm::dvec2 advance{0.0};
  };

  class Font {
  public:
    explicit Font(const std::string &path, int atlasSize = 1024);

    ~Font() noexcept;

    Font(const Font &) = delete;

    Font &operator=(const Font &) = delete;

    std::vector<ShapedGlyph> shape(const std::string &text);

    const GlyphInfo &glyph(std::uint32_t glyphId);

    [[nodiscard]] const GL::Texture &atlas() const noexcept { return m_atlas; }

    [[nodiscard]] double lineHeight() const noexcept { return m_lineHeight; }

    [[nodiscard]] double ascender() const noexcept { return m_ascender; }

    [[nodiscard]] double descender() const noexcept { return m_descender; }

    [[nodiscard]] double distanceFieldRangePixels() const noexcept { return m_distanceFieldRangePixels; }

    [[nodiscard]] GlyphDebugImage debugGlyphDistanceField(std::uint32_t glyphId);

  private:
    struct AtlasSlot {
      glm::ivec2 outerOriginPixels{0};
      glm::ivec2 innerOriginPixels{0};
    };

    struct GeneratedGlyphDistanceField {
      std::vector<float> pixels;
      glm::vec2 planeBoundsMin{0.0f};
      glm::vec2 planeBoundsMax{0.0f};
    };

    void generateGlyph(std::uint32_t glyphId);

    [[nodiscard]] GeneratedGlyphDistanceField buildGlyphDistanceField(std::uint32_t glyphId);

    void clearAtlasSlot(const AtlasSlot &atlasSlot) const;

    [[nodiscard]] GlyphInfo buildGlyphInfo(const AtlasSlot &atlasSlot,
                                           const GeneratedGlyphDistanceField &generatedGlyph) const;

    [[nodiscard]] AtlasSlot reserveAtlasSlot();

    [[nodiscard]] int glyphSlotSizePixels() const noexcept;

    FT_Library m_freetypeLibrary{};
    FT_Face m_freetypeFace{};
    hb_font_t *m_harfbuzzFont{};
    hb_buffer_t *m_harfbuzzBuffer{};
    msdfgen::FreetypeHandle *m_msdfgenFreetype{};
    msdfgen::FontHandle *m_msdfgenFont{};

    GL::Texture m_atlas;
    std::unordered_map<std::uint32_t, GlyphInfo> m_glyphs;

    int m_nextAtlasWriteXPixels{};
    int m_nextAtlasWriteYPixels{};
    int m_currentAtlasRowHeightPixels{};
    int m_glyphBitmapSizePixels{64};
    int m_atlasGutterPixels{2};
    double m_distanceFieldRangePixels{4.0};

    double m_lineHeight{};
    double m_ascender{};
    double m_descender{};
  };
} // Qadra::Core

#endif // QADRA_CORE_FONT_HPP
