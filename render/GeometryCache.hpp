#ifndef QADRA_GEOMETRY_CACHE_HPP
#define QADRA_GEOMETRY_CACHE_HPP

#include "LinePass.hpp"

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
    void sync ( const Cad::Document &document );
    void draw ( const Core::Camera &camera );

  private:
    static constexpr int kChunkSize = 4096;

    struct Chunk
    {
      std::vector<LinePass::Vertex> lineVertices;
      bool dirty = true;
    };

    std::vector<Chunk> m_chunks;
    std::size_t m_lastVersion = 0;

    LinePass m_linePass;
    std::vector<LinePass::Vertex> m_mergedLineVertices;

    bool m_needsUpload = true;
  };
} // namespace Qadra::Render

#endif // QADRA_GEOMETRY_CACHE_HPP
