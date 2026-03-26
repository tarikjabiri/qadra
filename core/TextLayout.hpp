#ifndef QADRA_TEXT_LAYOUT_HPP
#define QADRA_TEXT_LAYOUT_HPP

#include "ShapedGlyph.hpp"

#include <QList>

namespace Qadra::Core
{
  struct TextLayout
  {
    QList<ShapedGlyph> glyphs;
    double advanceWidth = 0.0;

    [[nodiscard]] bool empty () const noexcept { return glyphs.isEmpty (); }
  };
} // namespace Qadra::Core

#endif // QADRA_TEXT_LAYOUT_HPP
