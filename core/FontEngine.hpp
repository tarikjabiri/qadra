#ifndef QADRA_FONTENGINE_HPP
#define QADRA_FONTENGINE_HPP

#include <QHash>
#include <QString>
#include <ft2build.h>
#include FT_FREETYPE_H

namespace Qadra::Core
{
  class FontEngine
  {
  public:
    FontEngine ();

    ~FontEngine () noexcept;

    FontEngine ( const FontEngine & ) = delete;

    FontEngine &operator= ( const FontEngine & ) = delete;

    FT_Face face ( const QString &path );

    [[nodiscard]] FT_Library library () const noexcept { return m_library; }

    [[nodiscard]] const QHash<QString, FT_Face> &faces () const noexcept { return m_faces; }

  private:
    FT_Library m_library;
    QHash<QString, FT_Face> m_faces;
  };
} // namespace Qadra::Core

#endif // QADRA_FONTENGINE_HPP
