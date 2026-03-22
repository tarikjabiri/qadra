#include "Font.hpp"

#include <msdfgen-ext.h>
#include <msdfgen.h>

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace {
  constexpr double angleThresholdRadians = 3.0;

  [[nodiscard]] std::string fontError(const std::string &path, const char *reason) {
    return "Failed to initialize font '" + path + "': " + reason;
  }

  [[nodiscard]] msdfgen::Projection glyphProjection(const msdfgen::Shape::Bounds &bounds,
                                                    const int bitmapSize,
                                                    const msdfgen::Range pixelRange) {
    double left = bounds.l;
    double bottom = bounds.b;
    double right = bounds.r;
    double top = bounds.t;

    if (left >= right || bottom >= top) {
      left = 0.0;
      bottom = 0.0;
      right = 1.0;
      top = 1.0;
    }

    msdfgen::Vector2 frame(bitmapSize, bitmapSize);
    frame += 2.0 * pixelRange.lower;
    if (frame.x <= 0.0 || frame.y <= 0.0) {
      throw std::runtime_error("Configured MSDF range does not fit into the glyph bitmap");
    }

    msdfgen::Vector2 dimensions(right - left, top - bottom);
    if (dimensions.x <= 0.0) {
      dimensions.x = 1.0;
    }
    if (dimensions.y <= 0.0) {
      dimensions.y = 1.0;
    }

    msdfgen::Vector2 scale;
    msdfgen::Vector2 translate;
    if (dimensions.x * frame.y < dimensions.y * frame.x) {
      translate.set(.5 * (frame.x / frame.y * dimensions.y - dimensions.x) - left, -bottom);
      scale = msdfgen::Vector2(frame.y / dimensions.y);
    } else {
      translate.set(-left, .5 * (frame.y / frame.x * dimensions.x - dimensions.y) - bottom);
      scale = msdfgen::Vector2(frame.x / dimensions.x);
    }
    translate -= pixelRange.lower / scale;

    return msdfgen::Projection(scale, translate);
  }

  [[nodiscard]] Qadra::Core::GlyphInfo blankGlyphInfo(const std::pair<int, int> slotOrigin,
                                                      const Qadra::GL::Texture &atlas,
                                                      const int glyphBitmapSize) {
    const glm::vec2 uvMin(
      static_cast<float>(slotOrigin.first) / static_cast<float>(atlas.width()),
      static_cast<float>(slotOrigin.second) / static_cast<float>(atlas.height())
    );
    const glm::vec2 uvMax(
      static_cast<float>(slotOrigin.first + glyphBitmapSize) / static_cast<float>(atlas.width()),
      static_cast<float>(slotOrigin.second + glyphBitmapSize) / static_cast<float>(atlas.height())
    );

    return {
      .uvMin = uvMin,
      .uvMax = uvMax,
      .size = glm::vec2(0.0f),
      .bearing = glm::vec2(0.0f),
      .cached = true,
    };
  }
}

namespace Qadra::Core {
  Font::Font(const std::string &path, const int atlasSize)
    : m_atlas(atlasSize, atlasSize, GL_RGB8) {
    try {
      if (atlasSize <= 0) {
        throw std::runtime_error("Font atlas size must be positive");
      }
      if (m_glyphBitmapSize > atlasSize) {
        throw std::runtime_error("Glyph bitmap size exceeds the atlas size");
      }

      if (FT_Init_FreeType(&m_freetypeLibrary) != FT_Err_Ok) {
        throw std::runtime_error(fontError(path, "FreeType initialization failed"));
      }

      if (FT_New_Face(m_freetypeLibrary, path.c_str(), 0, &m_freetypeFace) != FT_Err_Ok) {
        throw std::runtime_error(fontError(path, "font face could not be loaded"));
      }

      m_harfbuzzFont = hb_ft_font_create_referenced(m_freetypeFace);
      if (!m_harfbuzzFont) {
        throw std::runtime_error(fontError(path, "HarfBuzz font creation failed"));
      }

      m_harfbuzzBuffer = hb_buffer_create();
      if (!m_harfbuzzBuffer) {
        throw std::runtime_error(fontError(path, "HarfBuzz buffer creation failed"));
      }

      const int fontUnitsPerEm = m_freetypeFace->units_per_EM > 0 ? m_freetypeFace->units_per_EM : 1024;
      hb_font_set_scale(m_harfbuzzFont, fontUnitsPerEm, fontUnitsPerEm);

      m_msdfgenFreetype = msdfgen::initializeFreetype();
      if (!m_msdfgenFreetype) {
        throw std::runtime_error(fontError(path, "msdfgen FreeType bridge initialization failed"));
      }

      m_msdfgenFont = msdfgen::loadFont(m_msdfgenFreetype, path.c_str());
      if (!m_msdfgenFont) {
        throw std::runtime_error(fontError(path, "msdfgen font face could not be loaded"));
      }

      msdfgen::FontMetrics metrics{};
      if (!msdfgen::getFontMetrics(metrics, m_msdfgenFont, msdfgen::FONT_SCALING_NONE)) {
        throw std::runtime_error(fontError(path, "font metrics could not be loaded"));
      }

      m_lineHeight = metrics.lineHeight;
      m_ascender = metrics.ascenderY;
      m_descender = metrics.descenderY;
    } catch (...) {
      if (m_msdfgenFont) {
        msdfgen::destroyFont(m_msdfgenFont);
        m_msdfgenFont = nullptr;
      }
      if (m_msdfgenFreetype) {
        msdfgen::deinitializeFreetype(m_msdfgenFreetype);
        m_msdfgenFreetype = nullptr;
      }
      if (m_harfbuzzBuffer) {
        hb_buffer_destroy(m_harfbuzzBuffer);
        m_harfbuzzBuffer = nullptr;
      }
      if (m_harfbuzzFont) {
        hb_font_destroy(m_harfbuzzFont);
        m_harfbuzzFont = nullptr;
      }
      if (m_freetypeFace) {
        FT_Done_Face(m_freetypeFace);
        m_freetypeFace = nullptr;
      }
      if (m_freetypeLibrary) {
        FT_Done_FreeType(m_freetypeLibrary);
        m_freetypeLibrary = nullptr;
      }
      throw;
    }
  }

  Font::~Font() noexcept {
    if (m_msdfgenFont) {
      msdfgen::destroyFont(m_msdfgenFont);
    }
    if (m_msdfgenFreetype) {
      msdfgen::deinitializeFreetype(m_msdfgenFreetype);
    }
    if (m_harfbuzzBuffer) {
      hb_buffer_destroy(m_harfbuzzBuffer);
    }
    if (m_harfbuzzFont) {
      hb_font_destroy(m_harfbuzzFont);
    }
    if (m_freetypeFace) {
      FT_Done_Face(m_freetypeFace);
    }
    if (m_freetypeLibrary) {
      FT_Done_FreeType(m_freetypeLibrary);
    }
  }

  std::vector<ShapedGlyph> Font::shape(const std::string &text) {
    hb_buffer_reset(m_harfbuzzBuffer);
    hb_buffer_add_utf8(m_harfbuzzBuffer, text.c_str(), static_cast<int>(text.size()), 0, static_cast<int>(text.size()));
    hb_buffer_guess_segment_properties(m_harfbuzzBuffer);
    hb_shape(m_harfbuzzFont, m_harfbuzzBuffer, nullptr, 0);

    unsigned int glyphCount = 0;
    const hb_glyph_info_t *glyphInfos = hb_buffer_get_glyph_infos(m_harfbuzzBuffer, &glyphCount);
    const hb_glyph_position_t *glyphPositions = hb_buffer_get_glyph_positions(m_harfbuzzBuffer, &glyphCount);

    std::vector<ShapedGlyph> shapedGlyphs;
    shapedGlyphs.reserve(glyphCount);
    for (unsigned int i = 0; i < glyphCount; ++i) {
      shapedGlyphs.push_back({
        .glyphId = glyphInfos[i].codepoint,
        .offset = glm::dvec2(glyphPositions[i].x_offset, glyphPositions[i].y_offset),
        .advance = glm::dvec2(glyphPositions[i].x_advance, glyphPositions[i].y_advance),
      });
    }

    return shapedGlyphs;
  }

  const GlyphInfo &Font::glyph(const std::uint32_t glyphId) {
    const auto glyphIterator = m_glyphs.find(glyphId);
    if (glyphIterator != m_glyphs.end() && glyphIterator->second.cached) {
      return glyphIterator->second;
    }

    generateGlyph(glyphId);
    return m_glyphs.at(glyphId);
  }

  void Font::generateGlyph(const std::uint32_t glyphId) {
    if (FT_Load_Glyph(m_freetypeFace, glyphId, FT_LOAD_NO_SCALE) != FT_Err_Ok) {
      throw std::runtime_error("Failed to load glyph metrics from FreeType");
    }

    const auto slotOrigin = reserveAtlasSlot();
    const FT_GlyphSlot glyphSlot = m_freetypeFace->glyph;
    if (glyphSlot->format != FT_GLYPH_FORMAT_OUTLINE || glyphSlot->outline.n_contours <= 0 || glyphSlot->outline.n_points <= 0) {
      const std::vector<float> blankBitmap(static_cast<std::size_t>(m_glyphBitmapSize) * m_glyphBitmapSize * 3, 0.0f);
      m_atlas.upload(
        slotOrigin.first,
        slotOrigin.second,
        m_glyphBitmapSize,
        m_glyphBitmapSize,
        GL_RGB,
        GL_FLOAT,
        blankBitmap.data()
      );

      m_glyphs[glyphId] = blankGlyphInfo(slotOrigin, m_atlas, m_glyphBitmapSize);
      return;
    }

    msdfgen::Shape shape;
    if (!msdfgen::loadGlyph(shape, m_msdfgenFont, msdfgen::GlyphIndex(glyphId), msdfgen::FONT_SCALING_NONE)) {
      throw std::runtime_error("Failed to load glyph outline for MSDF generation");
    }

    if (shape.contours.empty()) {
      const std::vector<float> blankBitmap(static_cast<std::size_t>(m_glyphBitmapSize) * m_glyphBitmapSize * 3, 0.0f);
      m_atlas.upload(
        slotOrigin.first,
        slotOrigin.second,
        m_glyphBitmapSize,
        m_glyphBitmapSize,
        GL_RGB,
        GL_FLOAT,
        blankBitmap.data()
      );

      m_glyphs[glyphId] = blankGlyphInfo(slotOrigin, m_atlas, m_glyphBitmapSize);
      return;
    }

    shape.normalize();
    msdfgen::edgeColoringSimple(shape, angleThresholdRadians, glyphId);

    const msdfgen::Range pixelRange(m_distanceFieldRange);
    const msdfgen::Shape::Bounds bounds = shape.getBounds();
    const msdfgen::Projection projection = glyphProjection(bounds, m_glyphBitmapSize, pixelRange);

    msdfgen::Bitmap<float, 3> bitmap(m_glyphBitmapSize, m_glyphBitmapSize);
    msdfgen::generateMSDF(bitmap, shape, projection, pixelRange);

    m_atlas.upload(
      slotOrigin.first,
      slotOrigin.second,
      m_glyphBitmapSize,
      m_glyphBitmapSize,
      GL_RGB,
      GL_FLOAT,
      static_cast<const float *>(bitmap)
    );

    const double planeLeft = projection.unprojectX(0.0);
    const double planeRight = projection.unprojectX(m_glyphBitmapSize);
    const double planeBottom = projection.unprojectY(0.0);
    const double planeTop = projection.unprojectY(m_glyphBitmapSize);

    m_glyphs[glyphId] = {
      .uvMin = glm::vec2(
        static_cast<float>(slotOrigin.first) / static_cast<float>(m_atlas.width()),
        static_cast<float>(slotOrigin.second) / static_cast<float>(m_atlas.height())
      ),
      .uvMax = glm::vec2(
        static_cast<float>(slotOrigin.first + m_glyphBitmapSize) / static_cast<float>(m_atlas.width()),
        static_cast<float>(slotOrigin.second + m_glyphBitmapSize) / static_cast<float>(m_atlas.height())
      ),
      .size = glm::vec2(
        static_cast<float>(planeRight - planeLeft),
        static_cast<float>(planeTop - planeBottom)
      ),
      .bearing = glm::vec2(
        static_cast<float>(planeLeft),
        static_cast<float>(planeTop)
      ),
      .cached = true,
    };
  }

  std::pair<int, int> Font::reserveAtlasSlot() {
    if (m_penX + m_glyphBitmapSize > m_atlas.width()) {
      m_penX = 0;
      m_penY += m_rowHeight;
      m_rowHeight = 0;
    }

    if (m_penY + m_glyphBitmapSize > m_atlas.height()) {
      throw std::runtime_error("Font atlas is full");
    }

    const std::pair<int, int> slotOrigin(m_penX, m_penY);
    m_penX += m_glyphBitmapSize;
    m_rowHeight = std::max(m_rowHeight, m_glyphBitmapSize);
    return slotOrigin;
  }
} // Qadra::Core
