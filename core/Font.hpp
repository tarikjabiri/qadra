#ifndef QADRA_CORE_FONT_HPP
#define QADRA_CORE_FONT_HPP

#include "FontEngine.hpp"
#include "GlyphAtlas.hpp"
#include "GlyphData.hpp"
#include "ShapedGlyph.hpp"
#include "TextLayout.hpp"
#include "TextShaper.hpp"
#include "gl/Texture.hpp"

namespace Qadra::Core
{
  class Font
  {
  public:
    Font ( FontEngine &engine, const QString &path, int size = 1024 );

    QList<ShapedGlyph> shape ( const QString &text ) const { return m_shaper.shape ( text ); }

    TextLayout layout ( const QString &text ) const;

    const GlyphData &glyph ( const std::uint32_t glyphId ) { return m_atlas.glyph ( glyphId ); }

    [[nodiscard]] const GL::Texture &texture () const noexcept { return m_atlas.texture (); }

    [[nodiscard]] double distanceFieldRange () const noexcept
    {
      return m_atlas.distanceFieldRange ();
    }

    [[nodiscard]] double lineHeight () const noexcept { return m_lineHeight; }

    [[nodiscard]] double ascender () const noexcept { return m_ascender; }

    [[nodiscard]] double descender () const noexcept { return m_descender; }

    [[nodiscard]] int unitsPerEm () const noexcept { return m_shaper.unitsPerEm (); }

  private:
    TextShaper m_shaper;
    GlyphAtlas m_atlas;
    double m_lineHeight{};
    double m_ascender{};
    double m_descender{};
  };
} // namespace Qadra::Core

#endif // QADRA_CORE_FONT_HPP
