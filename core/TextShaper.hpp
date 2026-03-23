#ifndef QADRA_TEXT_SHAPER_HPP
#define QADRA_TEXT_SHAPER_HPP

#include "FontEngine.hpp"
#include "ShapedGlyph.hpp"

#include <hb.h>

namespace Qadra::Core
{
  class TextShaper
  {
  public:
    TextShaper ( FontEngine &engine, const QString &path );
    ~TextShaper () noexcept;

    TextShaper ( const TextShaper & ) = delete;
    TextShaper &operator= ( const TextShaper & ) = delete;

    QList<ShapedGlyph> shape ( const QString &text ) const;

    [[nodiscard]] int unitsPerEm () const noexcept { return m_unitsPerEm; }

  private:
    hb_font_t *m_font{};
    int m_unitsPerEm{};
    hb_buffer_t *m_buffer{};
  };
} // namespace Qadra::Core

#endif // QADRA_TEXT_SHAPER_HPP
