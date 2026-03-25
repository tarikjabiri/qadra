#ifndef QADRA_GLYPH_ATLAS_HPP
#define QADRA_GLYPH_ATLAS_HPP

#include "FontEngine.hpp"
#include "GlyphData.hpp"
#include "gl/Texture.hpp"

#include <msdf-atlas-gen/msdf-atlas-gen.h>
#include <msdfgen/msdfgen-ext.h>

namespace Qadra::Core
{
  class GlyphAtlas
  {
  public:
    GlyphAtlas ( FontEngine &engine, const QString &path, int size = 1024 );
    ~GlyphAtlas () noexcept;

    GlyphAtlas ( const GlyphAtlas & ) = delete;
    GlyphAtlas &operator= ( const GlyphAtlas & ) = delete;

    const GlyphData &glyph ( std::uint32_t glyphId );

    [[nodiscard]] const GL::Texture &texture () const noexcept { return m_texture; }

    [[nodiscard]] static double distanceFieldRange () noexcept;

  private:
    void uploadAtlasToTexture ();

    using Generator = msdf_atlas::ImmediateAtlasGenerator<float, 3, msdf_atlas::msdfGenerator,
                                                          msdf_atlas::BitmapAtlasStorage<float, 3>>;
    using AtlasType = msdf_atlas::DynamicAtlas<Generator>;

    msdfgen::FontHandle *m_fontHandle{};
    std::unique_ptr<AtlasType> m_atlas;
    QHash<std::uint32_t, GlyphData> m_glyphs;
    GL::Texture m_texture;
    int m_unitsPerEm;
  };
} // namespace Qadra::Core

#endif // QADRA_GLYPH_ATLAS_HPP
