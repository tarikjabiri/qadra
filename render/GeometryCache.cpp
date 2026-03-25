#include "GeometryCache.hpp"

#include "Document.hpp"
#include <random>

#include <QFile>
#include <QTextStream>

namespace
{
  void buildTextQuads ( const Qadra::Entity::Text *text, Qadra::Core::Font &font,
                        const glm::vec4 &color, float depth,
                        std::vector<Qadra::Render::TextPass::Vertex> &out )
  {
    const auto shaped = font.shape ( text->text () );
    const double scale = text->height () / font.unitsPerEm ();
    const double cos = std::cos ( text->rotation () );
    const double sin = std::sin ( text->rotation () );

    auto transform = [&] ( const glm::dvec2 &local ) -> glm::vec2
    {
      return glm::vec2 ( text->position () + glm::dvec2 ( local.x * cos - local.y * sin,
                                                          local.x * sin + local.y * cos ) );
    };

    glm::dvec2 cursor ( 0.0 );

    for ( const auto &sg : shaped )
    {
      const auto &glyph = font.glyph ( sg.glyphId );

      if ( ! glyph.empty )
      {
        const glm::dvec2 offset = ( cursor + sg.offset ) * scale;
        const glm::dvec2 qMin = offset + glm::dvec2 ( glyph.quadMin ) * text->height ();
        const glm::dvec2 qMax = offset + glm::dvec2 ( glyph.quadMax ) * text->height ();

        const glm::vec2 p0 = transform ( glm::dvec2 ( qMin.x, qMin.y ) );
        const glm::vec2 p1 = transform ( glm::dvec2 ( qMax.x, qMin.y ) );
        const glm::vec2 p2 = transform ( glm::dvec2 ( qMax.x, qMax.y ) );
        const glm::vec2 p3 = transform ( glm::dvec2 ( qMin.x, qMax.y ) );

        const glm::vec2 uvMin = glyph.uvMin;
        const glm::vec2 uvMax = glyph.uvMax;

        out.push_back ( { p0, { uvMin.x, uvMin.y }, color, depth } );
        out.push_back ( { p1, { uvMax.x, uvMin.y }, color, depth } );
        out.push_back ( { p2, { uvMax.x, uvMax.y }, color, depth } );

        out.push_back ( { p0, { uvMin.x, uvMin.y }, color, depth } );
        out.push_back ( { p2, { uvMax.x, uvMax.y }, color, depth } );
        out.push_back ( { p3, { uvMin.x, uvMax.y }, color, depth } );
      }

      cursor += sg.advance;
    }
  }
} // namespace

namespace Qadra::Render
{
  void GeometryCache::init ( const QString &shaderDir )
  {
    auto load = [&] ( const QString &filename )
    {
      QFile file ( shaderDir + "/" + filename );
      if ( ! file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
        throw std::runtime_error ( "Failed to open shader: " + filename.toStdString () );
      return QTextStream ( &file ).readAll ();
    };

    m_linePass.init ( load ( "line.vertex.glsl" ), load ( "line.fragment.glsl" ) );
    m_textPass.init ( load ( "text.vertex.glsl" ), load ( "text.fragment.glsl" ) );
  }

  void GeometryCache::sync ( const Cad::Document &document, Core::Font &font )
  {
    const auto &order = document.drawOrder ();
    const std::size_t totalChunks = ( order.size () + kChunkSize - 1 ) / kChunkSize;

    if ( m_lastVersion == document.version () ) return;

    const std::size_t dirtyFrom = document.dirtyFrom ();
    const std::size_t firstDirtyChunk = dirtyFrom / kChunkSize;

    m_chunks.resize ( totalChunks );

    for ( std::size_t c = firstDirtyChunk; c < totalChunks; ++c ) m_chunks[c].dirty = true;

    const float maxDepth = static_cast<float> ( order.size () + 1 );

    for ( std::size_t c = firstDirtyChunk; c < totalChunks; ++c )
    {
      auto &[lineVertices, textVertices, dirty] = m_chunks[c];
      if ( ! dirty ) continue;

      lineVertices.clear ();
      textVertices.clear ();

      const int start = static_cast<int> ( c * kChunkSize );
      const int end = std::min ( start + kChunkSize, static_cast<int> ( order.size () ) );

      for ( int i = start; i < end; ++i )
      {
        const auto *entity = document.find ( order[i] );
        if ( ! entity ) continue;

        // Later entities should appear on top. With the default depth test,
        // smaller depth values are closer, so invert the draw-order mapping.
        const float depth = 1.0f - static_cast<float> ( i + 1 ) / maxDepth;
        std::random_device rd;
        std::mt19937 gen ( rd () );
        std::uniform_real_distribution<float> dist ( 0.0f, 1.0f );

        const glm::vec4 color{ dist ( gen ), dist ( gen ), dist ( gen ), 1.0f };

        switch ( entity->type () )
        {
          case Entity::EntityType::Line:
          {
            const auto *line = static_cast<const Entity::Line *> ( entity );
            lineVertices.push_back ( { line->start (), color, depth } );
            lineVertices.push_back ( { line->end (), color, depth } );
            break;
          }
          case Entity::EntityType::Text:
          {
            const auto *text = static_cast<const Entity::Text *> ( entity );
            buildTextQuads ( text, font, color, depth, textVertices );
            break;
          }
        }
      }

      dirty = false;
    }

    m_lastVersion = document.version ();
    m_needsUpload = true;
    document.resetDirty ();
  }

  void GeometryCache::draw ( const Core::Camera &camera, const Core::Font &font )
  {
    if ( m_needsUpload )
    {
      m_mergedLineVertices.clear ();
      m_mergedTextVertices.clear ();

      for ( const auto &chunk : m_chunks )
      {
        m_mergedLineVertices.insert ( m_mergedLineVertices.end (), chunk.lineVertices.begin (),
                                      chunk.lineVertices.end () );
        m_mergedTextVertices.insert ( m_mergedTextVertices.end (), chunk.textVertices.begin (),
                                      chunk.textVertices.end () );
      }

      m_linePass.upload ( m_mergedLineVertices );
      m_textPass.upload ( m_mergedTextVertices );
      m_needsUpload = false;
    }

    m_linePass.render ( camera );
    m_textPass.render ( camera, font.texture (), font.distanceFieldRange () );
  }
} // namespace Qadra::Render
