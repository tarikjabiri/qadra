#include "Font.hpp"

namespace Qadra::Core
{
  Font::Font ( FontEngine &engine, const QString &path, const int size )
      : m_shaper ( engine, path ), m_atlas ( engine, path, size )
  {
    const FT_Face face = engine.face ( path );

    m_lineHeight = face->height > 0 ? face->height : m_shaper.unitsPerEm ();
    m_ascender = face->ascender > 0 ? face->ascender : m_shaper.unitsPerEm ();
    m_descender = face->descender < 0 ? face->descender : 0.0;
  }
} // namespace Qadra::Core
