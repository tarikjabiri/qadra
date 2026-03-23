#include "Font.hpp"

#include <msdfgen-ext.h>
#include <msdfgen.h>

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

#include "freetype/ftoutln.h"

namespace {
  constexpr double angleThresholdRadians = 3.0;

  struct ScopedHarfbuzzBuffer {
    ScopedHarfbuzzBuffer()
      : handle(hb_buffer_create()) {
    }

    hb_buffer_t *handle{};

    ~ScopedHarfbuzzBuffer() {
      if (handle) {
        hb_buffer_destroy(handle);
      }
    }

    ScopedHarfbuzzBuffer(const ScopedHarfbuzzBuffer &) = delete;
    ScopedHarfbuzzBuffer &operator=(const ScopedHarfbuzzBuffer &) = delete;
    ScopedHarfbuzzBuffer(ScopedHarfbuzzBuffer &&) = delete;
    ScopedHarfbuzzBuffer &operator=(ScopedHarfbuzzBuffer &&) = delete;
  };

  struct GlyphProjectionData {
    msdfgen::Projection projection;
    msdfgen::Range shapeRange;
  };

  [[nodiscard]] std::string fontError(const std::string &path, const char *reason) {
    return "Failed to initialize font '" + path + "': " + reason;
  }

  [[nodiscard]] GlyphProjectionData createGlyphProjectionData(const msdfgen::Shape::Bounds &bounds,
                                                              const int glyphBitmapSizePixels,
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

    msdfgen::Vector2 frame(glyphBitmapSizePixels, glyphBitmapSizePixels);
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

    const double minimumScale = std::min(scale.x, scale.y);
    if (minimumScale <= 0.0) {
      throw std::runtime_error("Glyph projection produced a non-positive scale");
    }

    return {
      .projection = msdfgen::Projection(scale, translate),
      .shapeRange = pixelRange / minimumScale,
    };
  }

  [[nodiscard]] std::vector<float> blankGlyphPixels(const int glyphBitmapSizePixels) {
    return std::vector<float>(static_cast<std::size_t>(glyphBitmapSizePixels) * glyphBitmapSizePixels * 3, 0.0f);
  }

}

namespace Qadra::Core {
  Font::Font(const std::string &path, const int atlasSize)
    : m_atlas(atlasSize, atlasSize, GL_RGB8) {
    try {
      if (atlasSize <= 0) {
        throw std::runtime_error("Font atlas size must be positive");
      }
      if ((m_glyphBitmapSizePixels + 2 * m_atlasGutterPixels) > atlasSize) {
        throw std::runtime_error("Glyph slot size exceeds the atlas size");
      }

      if (FT_Init_FreeType(&m_freetypeLibrary) != FT_Err_Ok) {
        throw std::runtime_error(fontError(path, "FreeType initialization failed"));
      }

      if (FT_New_Face(m_freetypeLibrary, path.c_str(), 0, &m_freetypeFace) != FT_Err_Ok) {
        throw std::runtime_error(fontError(path, "font face could not be loaded"));
      }

      hb_blob_t *harfbuzzBlob = hb_blob_create_from_file_or_fail(path.c_str());
      if (!harfbuzzBlob) {
        throw std::runtime_error(fontError(path, "HarfBuzz font blob could not be loaded"));
      }

      hb_face_t *harfbuzzFace = hb_face_create(harfbuzzBlob, 0);
      hb_blob_destroy(harfbuzzBlob);
      if (!harfbuzzFace || hb_face_is_immutable(harfbuzzFace)) {
        if (harfbuzzFace) {
          hb_face_destroy(harfbuzzFace);
        }
        throw std::runtime_error(fontError(path, "HarfBuzz face creation failed"));
      }

      m_harfbuzzFont = hb_font_create(harfbuzzFace);
      hb_face_destroy(harfbuzzFace);
      if (!m_harfbuzzFont || hb_font_is_immutable(m_harfbuzzFont)) {
        if (m_harfbuzzFont) {
          hb_font_destroy(m_harfbuzzFont);
          m_harfbuzzFont = nullptr;
        }
        throw std::runtime_error(fontError(path, "HarfBuzz font creation failed"));
      }

      const int fontUnitsPerEm = m_freetypeFace->units_per_EM > 0 ? m_freetypeFace->units_per_EM : 1024;
      hb_font_set_scale(m_harfbuzzFont, fontUnitsPerEm, fontUnitsPerEm);
      hb_font_make_immutable(m_harfbuzzFont);

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
      m_maximumGlyphBoundsMinimumY = m_descender;
      m_maximumGlyphBoundsMaximumY = m_ascender;
      computeMaximumGlyphBounds();
    } catch (...) {
      if (m_msdfgenFont) {
        msdfgen::destroyFont(m_msdfgenFont);
        m_msdfgenFont = nullptr;
      }
      if (m_msdfgenFreetype) {
        msdfgen::deinitializeFreetype(m_msdfgenFreetype);
        m_msdfgenFreetype = nullptr;
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

  std::vector<ShapedGlyph> Font::shape(const std::string &text) const {
    if (text.empty()) {
      return {};
    }

    const ScopedHarfbuzzBuffer harfbuzzBuffer;
    if (!harfbuzzBuffer.handle) {
      throw std::runtime_error("HarfBuzz buffer creation failed");
    }

    hb_buffer_add_utf8(harfbuzzBuffer.handle, text.c_str(), static_cast<int>(text.size()), 0, static_cast<int>(text.size()));
    hb_buffer_guess_segment_properties(harfbuzzBuffer.handle);
    hb_shape(m_harfbuzzFont, harfbuzzBuffer.handle, nullptr, 0);

    unsigned int glyphCount = 0;
    const hb_glyph_info_t *glyphInfos = hb_buffer_get_glyph_infos(harfbuzzBuffer.handle, &glyphCount);
    const hb_glyph_position_t *glyphPositions = hb_buffer_get_glyph_positions(harfbuzzBuffer.handle, &glyphCount);

    std::vector<ShapedGlyph> shapedGlyphs;
    shapedGlyphs.reserve(glyphCount);
    for (unsigned int i = 0; i < glyphCount; ++i) {
      shapedGlyphs.push_back({
        .glyphId = glyphInfos[i].codepoint,
        .cluster = glyphInfos[i].cluster,
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
    const GeneratedGlyphDistanceField generatedGlyph = buildGlyphDistanceField(glyphId);
    const bool hasPlaneBounds = generatedGlyph.planeBoundsMax.x > generatedGlyph.planeBoundsMin.x &&
                                generatedGlyph.planeBoundsMax.y > generatedGlyph.planeBoundsMin.y;
    const bool hasInkBounds = generatedGlyph.inkBoundsMax.x > generatedGlyph.inkBoundsMin.x &&
                              generatedGlyph.inkBoundsMax.y > generatedGlyph.inkBoundsMin.y;
    if (!hasPlaneBounds && !hasInkBounds) {
      cacheEmptyGlyph(glyphId);
      return;
    }

    const AtlasSlot atlasSlot = reserveAtlasSlot(
      glm::ivec2(m_glyphBitmapSizePixels, m_glyphBitmapSizePixels),
      m_atlas,
      m_atlasCursor
    );
    clearAtlasSlot(atlasSlot);

    m_atlas.upload(
      atlasSlot.innerOriginPixels.x,
      atlasSlot.innerOriginPixels.y,
      m_glyphBitmapSizePixels,
      m_glyphBitmapSizePixels,
      GL_RGB,
      GL_FLOAT,
      generatedGlyph.pixels.data()
    );

    m_glyphs[glyphId] = buildGlyphInfo(atlasSlot, generatedGlyph);
  }

  Font::GeneratedGlyphDistanceField Font::emptyGlyphDistanceField() const {
    return {
      .pixels = blankGlyphPixels(m_glyphBitmapSizePixels),
      .planeBoundsMin = glm::vec2(0.0f),
      .planeBoundsMax = glm::vec2(0.0f),
      .inkBoundsMin = glm::vec2(0.0f),
      .inkBoundsMax = glm::vec2(0.0f),
    };
  }

  void Font::cacheEmptyGlyph(const std::uint32_t glyphId) {
    m_glyphs[glyphId] = {
      .cached = true,
    };
  }

  void Font::computeMaximumGlyphBounds() {
    double minimumY = std::numeric_limits<double>::infinity();
    double maximumY = -std::numeric_limits<double>::infinity();
    bool foundVisibleGlyph = false;

    for (FT_UInt glyphIndex = 0; glyphIndex < m_freetypeFace->num_glyphs; ++glyphIndex) {
      if (FT_Load_Glyph(m_freetypeFace, glyphIndex, FT_LOAD_NO_SCALE) != FT_Err_Ok) {
        continue;
      }

      const FT_GlyphSlot glyphSlot = m_freetypeFace->glyph;
      if (glyphSlot->metrics.height <= 0) {
        continue;
      }

      double glyphTop = 0.0;
      double glyphBottom = 0.0;
      if (glyphSlot->format == FT_GLYPH_FORMAT_OUTLINE &&
          glyphSlot->outline.n_contours > 0 &&
          glyphSlot->outline.n_points > 0) {
        FT_BBox outlineBounds{};
        FT_Outline_Get_CBox(&glyphSlot->outline, &outlineBounds);
        glyphTop = static_cast<double>(outlineBounds.yMax);
        glyphBottom = static_cast<double>(outlineBounds.yMin);
      } else {
        glyphTop = static_cast<double>(glyphSlot->metrics.horiBearingY);
        glyphBottom = glyphTop - static_cast<double>(glyphSlot->metrics.height);
      }

      if (glyphTop <= glyphBottom) {
        continue;
      }

      minimumY = std::min(minimumY, glyphBottom);
      maximumY = std::max(maximumY, glyphTop);
      foundVisibleGlyph = true;
    }

    if (foundVisibleGlyph) {
      m_maximumGlyphBoundsMinimumY = minimumY;
      m_maximumGlyphBoundsMaximumY = maximumY;
    }
  }

  Font::GeneratedGlyphDistanceField Font::buildGlyphDistanceField(const std::uint32_t glyphId) const {
    if (FT_Load_Glyph(m_freetypeFace, glyphId, FT_LOAD_NO_SCALE) != FT_Err_Ok) {
      throw std::runtime_error("Failed to load glyph metrics from FreeType");
    }

    const FT_GlyphSlot glyphSlot = m_freetypeFace->glyph;
    if (glyphSlot->format != FT_GLYPH_FORMAT_OUTLINE || glyphSlot->outline.n_contours <= 0 || glyphSlot->outline.n_points <= 0) {
      return emptyGlyphDistanceField();
    }

    msdfgen::Shape shape;
    if (!msdfgen::loadGlyph(shape, m_msdfgenFont, msdfgen::GlyphIndex(glyphId), msdfgen::FONT_SCALING_NONE)) {
      throw std::runtime_error("Failed to load glyph outline for MSDF generation");
    }

    if (shape.contours.empty()) {
      return emptyGlyphDistanceField();
    }

    shape.normalize();
    msdfgen::edgeColoringSimple(shape, angleThresholdRadians, glyphId);
    const msdfgen::Shape::Bounds shapeBounds = shape.getBounds();

    const msdfgen::Range distanceFieldRangePixels(m_distanceFieldRangePixels);
    const GlyphProjectionData projectionData = createGlyphProjectionData(
      shapeBounds,
      m_glyphBitmapSizePixels,
      distanceFieldRangePixels
    );

    FT_BBox outlineBounds{};
    FT_Outline_Get_CBox(&glyphSlot->outline, &outlineBounds);

    msdfgen::Bitmap<float, 3> bitmap(m_glyphBitmapSizePixels, m_glyphBitmapSizePixels);
    msdfgen::generateMSDF(bitmap, shape, msdfgen::SDFTransformation(projectionData.projection, projectionData.shapeRange));

    std::vector<float> pixels(static_cast<std::size_t>(m_glyphBitmapSizePixels) * m_glyphBitmapSizePixels * 3);
    std::copy_n(static_cast<const float *>(bitmap), pixels.size(), pixels.data());

    return {
      .pixels = std::move(pixels),
      .planeBoundsMin = glm::vec2(
        static_cast<float>(projectionData.projection.unprojectX(0.0)),
        static_cast<float>(projectionData.projection.unprojectY(0.0))
      ),
      .planeBoundsMax = glm::vec2(
        static_cast<float>(projectionData.projection.unprojectX(m_glyphBitmapSizePixels)),
        static_cast<float>(projectionData.projection.unprojectY(m_glyphBitmapSizePixels))
      ),
      .inkBoundsMin = glm::vec2(
        static_cast<float>(outlineBounds.xMin),
        static_cast<float>(outlineBounds.yMin)
      ),
      .inkBoundsMax = glm::vec2(
        static_cast<float>(outlineBounds.xMax),
        static_cast<float>(outlineBounds.yMax)
      ),
    };
  }

  void Font::clearAtlasSlot(const AtlasSlot &atlasSlot) const {
    const int slotWidthPixels = atlasSlot.contentSizePixels.x + 2 * m_atlasGutterPixels;
    const int slotHeightPixels = atlasSlot.contentSizePixels.y + 2 * m_atlasGutterPixels;
    const std::vector<float> clearPixels(
      static_cast<std::size_t>(slotWidthPixels) * static_cast<std::size_t>(slotHeightPixels) * 3,
      0.0f
    );

    m_atlas.upload(
      atlasSlot.outerOriginPixels.x,
      atlasSlot.outerOriginPixels.y,
      slotWidthPixels,
      slotHeightPixels,
      GL_RGB,
      GL_FLOAT,
      clearPixels.data()
    );
  }

  GlyphInfo Font::buildGlyphInfo(const AtlasSlot &atlasSlot,
                                 const GeneratedGlyphDistanceField &generatedGlyph) const {
    const float atlasWidth = static_cast<float>(m_atlas.width());
    const float atlasHeight = static_cast<float>(m_atlas.height());
    const float halfTexelX = 0.5f / atlasWidth;
    const float halfTexelY = 0.5f / atlasHeight;

    const glm::vec2 uploadUvMin(
      static_cast<float>(atlasSlot.innerOriginPixels.x) / atlasWidth,
      static_cast<float>(atlasSlot.innerOriginPixels.y) / atlasHeight
    );
    const glm::vec2 uploadUvMax(
      static_cast<float>(atlasSlot.innerOriginPixels.x + atlasSlot.contentSizePixels.x) / atlasWidth,
      static_cast<float>(atlasSlot.innerOriginPixels.y + atlasSlot.contentSizePixels.y) / atlasHeight
    );

    return {
      .planeBoundsMin = generatedGlyph.planeBoundsMin,
      .planeBoundsMax = generatedGlyph.planeBoundsMax,
      .inkBoundsMin = generatedGlyph.inkBoundsMin,
      .inkBoundsMax = generatedGlyph.inkBoundsMax,
      .uvMin = uploadUvMin + glm::vec2(halfTexelX, halfTexelY),
      .uvMax = uploadUvMax - glm::vec2(halfTexelX, halfTexelY),
      .cached = true,
    };
  }

  Font::AtlasSlot Font::reserveAtlasSlot(const glm::ivec2 &contentSizePixels,
                                         const GL::Texture &atlas,
                                         AtlasWriteCursor &cursor) const {
    const int slotWidthPixels = contentSizePixels.x + 2 * m_atlasGutterPixels;
    const int slotHeightPixels = contentSizePixels.y + 2 * m_atlasGutterPixels;
    if (slotWidthPixels > atlas.width() || slotHeightPixels > atlas.height()) {
      throw std::runtime_error("Glyph bitmap does not fit into the font atlas");
    }
    if (cursor.nextWriteXPixels + slotWidthPixels > atlas.width()) {
      cursor.nextWriteXPixels = 0;
      cursor.nextWriteYPixels += cursor.currentRowHeightPixels;
      cursor.currentRowHeightPixels = 0;
    }

    if (cursor.nextWriteYPixels + slotHeightPixels > atlas.height()) {
      throw std::runtime_error("Font atlas is full");
    }

    const AtlasSlot atlasSlot{
      .outerOriginPixels = glm::ivec2(cursor.nextWriteXPixels, cursor.nextWriteYPixels),
      .innerOriginPixels = glm::ivec2(cursor.nextWriteXPixels + m_atlasGutterPixels,
                                      cursor.nextWriteYPixels + m_atlasGutterPixels),
      .contentSizePixels = contentSizePixels,
    };

    cursor.nextWriteXPixels += slotWidthPixels;
    cursor.currentRowHeightPixels = std::max(cursor.currentRowHeightPixels, slotHeightPixels);
    return atlasSlot;
  }
} // Qadra::Core
