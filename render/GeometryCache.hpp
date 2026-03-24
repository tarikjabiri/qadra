#ifndef QADRA_GEOMETRY_CACHE_HPP
#define QADRA_GEOMETRY_CACHE_HPP

#include "Font.hpp"
#include "LinePass.hpp"
#include "TextPass.hpp"

namespace Qadra::Cad
{
  class Document;
} // namespace Qadra::Cad

namespace Qadra::Render
{
  class GeometryCache
  {
  public:
    GeometryCache () = default;

    void init ( const QString &shaderDir );
    void sync ( const Cad::Document &document, Core::Font &font );
    void draw ( const Core::Camera &camera, const Core::Font &font );

  private:
    static constexpr int kChunkSize = 4096;

    struct Chunk
    {
      std::vector<LinePass::Vertex> lineVertices;
      std::vector<TextPass::Vertex> textVertices;
      bool dirty = true;
    };

    std::vector<Chunk> m_chunks;
    std::size_t m_lastVersion = 0;

    LinePass m_linePass;
    TextPass m_textPass;

    std::vector<LinePass::Vertex> m_mergedLineVertices;
    std::vector<TextPass::Vertex> m_mergedTextVertices;

    bool m_needsUpload = true;
  };
} // namespace Qadra::Render

#endif // QADRA_GEOMETRY_CACHE_HPP
