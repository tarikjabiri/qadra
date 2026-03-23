#include "FontEngine.hpp"

#include <stdexcept>

namespace Qadra::Core
{
  FontEngine::FontEngine ()
  {
    if ( FT_Init_FreeType ( &m_library ) != FT_Err_Ok )
    {
      throw std::runtime_error ( "Failed to initialize FreeType" );
    }
  }

  FontEngine::~FontEngine () noexcept
  {
    for ( const auto &face : m_faces )
    {
      FT_Done_Face ( face );
    }

    if ( m_library )
    {
      FT_Done_FreeType ( m_library );
    }
  }

  FT_Face FontEngine::face ( const QString &path )
  {
    if ( const auto it = m_faces.find ( path ); it != m_faces.end () )
    {
      return it.value ();
    }

    FT_Face face{};
    if ( const auto &bytes = path.toUtf8 ();
         FT_New_Face ( m_library, bytes.constData (), 0, &face ) != FT_Err_Ok )
    {
      throw std::runtime_error (
          QStringLiteral ( "Failed to load font: %1" ).arg ( path ).toStdString () );
    }

    m_faces.insert ( path, face );
    return face;
  }
} // namespace Qadra::Core
