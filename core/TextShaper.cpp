#include "TextShaper.hpp"

#include "hb-ft.h"

#include <stdexcept>

namespace Qadra::Core
{
  TextShaper::TextShaper ( FontEngine &engine, const QString &path )
  {
    const FT_Face ftFace = engine.face ( path );

    m_unitsPerEm = ftFace->units_per_EM > 0 ? ftFace->units_per_EM : 1024;

    hb_face_t *hbFace = hb_ft_face_create_referenced ( ftFace );
    if ( ! hbFace )
    {
      throw std::runtime_error ( "Failed to create HarfBuzz face: " + path.toStdString () );
    }

    m_font = hb_font_create ( hbFace );
    hb_face_destroy ( hbFace );
    if ( ! m_font )
    {
      throw std::runtime_error ( "Failed to create HarfBuzz font: " + path.toStdString () );
    }

    hb_font_set_scale ( m_font, m_unitsPerEm, m_unitsPerEm );
    hb_font_make_immutable ( m_font );

    m_buffer = hb_buffer_create ();
    if ( ! hb_buffer_allocation_successful ( m_buffer ) )
    {
      hb_buffer_destroy ( m_buffer );
      m_buffer = nullptr;
      throw std::runtime_error ( "Failed to create HarfBuzz buffer" );
    }

    hb_buffer_set_cluster_level ( m_buffer, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS );
  }

  TextShaper::~TextShaper () noexcept
  {
    if ( m_font )
    {
      hb_font_destroy ( m_font );
    }

    if ( m_buffer )
    {
      hb_buffer_destroy ( m_buffer );
    }
  }

  QList<ShapedGlyph> TextShaper::shape ( const QString &text ) const
  {
    if ( text.isEmpty () )
    {
      return {};
    }

    const QByteArray utf8 = text.toUtf8 ();

    hb_buffer_reset ( m_buffer );
    hb_buffer_add_utf8 ( m_buffer, utf8.constData (), utf8.size (), 0, utf8.size () );
    hb_buffer_guess_segment_properties ( m_buffer );
    hb_shape ( m_font, m_buffer, nullptr, 0 );

    unsigned int count = 0;
    const hb_glyph_info_t *infos = hb_buffer_get_glyph_infos ( m_buffer, &count );
    const hb_glyph_position_t *positions = hb_buffer_get_glyph_positions ( m_buffer, &count );

    QList<ShapedGlyph> result;
    result.reserve ( count );
    for ( unsigned int i = 0; i < count; ++i )
    {
      const auto position = positions[i];
      result.push_back ( {
          .glyphId = infos[i].codepoint,
          .cluster = infos[i].cluster,
          .offset = glm::dvec2 ( position.x_offset, position.y_offset ),
          .advance = glm::dvec2 ( position.x_advance, position.y_advance ),
      } );
    }

    return result;
  }
} // namespace Qadra::Core
