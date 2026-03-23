#include "GlyphAtlas.hpp"

namespace Qadra::Core
{
  static constexpr double kDistanceFieldRange = 4.0;
  static constexpr double kGlyphScale = 32.0;
  static constexpr double kMiterLimit = 1.0;
  static constexpr double kMaxCornerAngle = 3.0;

  GlyphAtlas::GlyphAtlas ( FontEngine &engine, const QString &path, const int size )
      : m_texture ( size, size, GL_RGB8 )
  {
    const FT_Face face = engine.face ( path );
    m_fontHandle = msdfgen::adoptFreetypeFont ( face );

    m_atlas = std::make_unique<AtlasType> ( size );
  }

  GlyphAtlas::~GlyphAtlas () noexcept
  {
    if ( m_fontHandle )
    {
      msdfgen::destroyFont ( m_fontHandle );
    }
  }

  const GlyphData &GlyphAtlas::glyph ( std::uint32_t glyphId )
  {
    if ( const auto it = m_glyphs.find ( glyphId ); it != m_glyphs.end () )
    {
      return it.value ();
    }

    msdf_atlas::GlyphGeometry glyphGeom;
    if ( ! glyphGeom.load ( m_fontHandle, 1.0, msdfgen::GlyphIndex ( glyphId ) ) )
    {
      m_glyphs.insert ( glyphId, GlyphData{} );
      return m_glyphs[glyphId];
    }

    if ( glyphGeom.isWhitespace () )
    {
      m_glyphs.insert ( glyphId, GlyphData{} );
      return m_glyphs[glyphId];
    }

    glyphGeom.edgeColoring ( msdfgen::edgeColoringInkTrap, kMaxCornerAngle, 0 );
    glyphGeom.wrapBox ( kGlyphScale, kDistanceFieldRange / kGlyphScale, kMiterLimit );

    if ( m_atlas->add ( &glyphGeom, 1 ) != AtlasType::NO_CHANGE )
    {
      uploadAtlasToTexture ();
    }

    double al, ab, ar, at;
    double pl, pb, pr, pt;
    glyphGeom.getQuadAtlasBounds ( al, ab, ar, at );
    glyphGeom.getQuadPlaneBounds ( pl, pb, pr, pt );

    const auto size = glm::dvec2 ( m_texture.width (), m_texture.height () );

    GlyphData data;
    data.quadMin = glm::vec2 ( glm::dvec2 ( pl, pb ) );
    data.quadMax = glm::vec2 ( glm::dvec2 ( pr, pt ) );
    data.uvMin = glm::vec2 ( glm::dvec2 ( al, ab ) / size );
    data.uvMax = glm::vec2 ( glm::dvec2 ( ar, at ) / size );
    data.empty = false;

    m_glyphs.insert ( glyphId, data );
    return m_glyphs[glyphId];
  }

  double GlyphAtlas::distanceFieldRange () noexcept { return kDistanceFieldRange; }

  void GlyphAtlas::uploadAtlasToTexture ()
  {
    const auto &storage = m_atlas->atlasGenerator ().atlasStorage ();
    const auto bitmap = storage.operator msdfgen::BitmapConstRef<msdfgen::byte, 3> ();

    if ( bitmap.width != m_texture.width () || bitmap.height != m_texture.height () )
    {
      m_texture = GL::Texture ( bitmap.width, bitmap.height, GL_RGB8 );
    }

    m_texture.upload ( 0, 0, bitmap.width, bitmap.height, GL_RGB, GL_UNSIGNED_BYTE, bitmap.pixels );
  }
} // namespace Qadra::Core
