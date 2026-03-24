#include "GeometryCache.hpp"

#include "Document.hpp"

#include <QFile>
#include <QTextStream>

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
  }

  void GeometryCache::sync ( const Cad::Document &document )
  {
    const auto &order = document.drawOrder ();
    const std::size_t totalChunks = ( order.size () + kChunkSize - 1 ) / kChunkSize;

    if ( m_lastVersion == document.version () ) return;

    const std::size_t dirtyFrom = document.dirtyFrom ();
    const std::size_t firstDirtyChunk = dirtyFrom / kChunkSize;

    m_chunks.resize ( totalChunks );

    for ( std::size_t c = firstDirtyChunk; c < totalChunks; ++c ) m_chunks[c].dirty = true;

    const float maxDepth = static_cast<float> ( order.size () );

    for ( std::size_t c = firstDirtyChunk; c < totalChunks; ++c )
    {
      auto &[lineVertices, dirty] = m_chunks[c];
      if ( ! dirty ) continue;

      lineVertices.clear ();

      const int start = static_cast<int> ( c * kChunkSize );
      const int end = std::min ( start + kChunkSize, static_cast<int> ( order.size () ) );

      for ( int i = start; i < end; ++i )
      {
        const auto *entity = document.find ( order[i] );
        if ( ! entity ) continue;

        const float depth = static_cast<float> ( i ) / maxDepth;
        constexpr glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };

        switch ( entity->type () )
        {
          case Entity::EntityType::Line:
          {
            const auto *line = static_cast<const Entity::Line *> ( entity );
            lineVertices.push_back ( { line->start (), color, depth } );
            lineVertices.push_back ( { line->end (), color, depth } );
            break;
          }
          default:
            break;
        }
      }

      dirty = false;
    }

    m_lastVersion = document.version ();
    m_needsUpload = true;
    document.resetDirty ();
  }

  void GeometryCache::draw ( const Core::Camera &camera )
  {
    if ( m_needsUpload )
    {
      m_mergedLineVertices.clear ();
      for ( const auto &[lineVertices, dirty] : m_chunks )
        m_mergedLineVertices.insert ( m_mergedLineVertices.end (), lineVertices.begin (),
                                      lineVertices.end () );

      m_linePass.upload ( m_mergedLineVertices );
      m_needsUpload = false;
    }

    m_linePass.render ( camera );
  }
} // namespace Qadra::Render
