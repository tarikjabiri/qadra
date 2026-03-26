#ifndef QADRA_RENDER_SCENE_HPP
#define QADRA_RENDER_SCENE_HPP

#include "Document.hpp"
#include "LinePass.hpp"
#include "TextPass.hpp"
#include "VertexBatch.hpp"
#include "VisibleDrawList.hpp"

#include <unordered_map>
#include <vector>

namespace Qadra::Render
{
  class RenderScene
  {
  public:
    void init ();
    void sync ( const Cad::Document &document, Core::Font &font );
    void draw ( const Core::Camera &camera, const Core::Font &font ) const;

  private:
    static constexpr std::size_t kMaxEntitiesPerPage = 512;
    static constexpr std::size_t kMaxLineVerticesPerPage = 4096;
    static constexpr std::size_t kMaxTextVerticesPerPage = 24576;
    static constexpr std::size_t kMaxOverlayEntities = 512;
    static constexpr std::size_t kMaxOverlayLineVertices = 16384;
    static constexpr std::size_t kMaxOverlayTextVertices = 131072;

    enum class StorageLocation
    {
      Main,
      Overlay,
    };

    struct Page
    {
      Math::BoxAABB bbox;
      GLint lineFirst = 0;
      GLsizei lineCount = 0;
      GLint textFirst = 0;
      GLsizei textCount = 0;
    };

    struct Placement
    {
      StorageLocation storage = StorageLocation::Main;
      Math::BoxAABB bbox;
      GLint lineFirst = 0;
      GLsizei lineCount = 0;
      GLint textFirst = 0;
      GLsizei textCount = 0;
    };

    struct GeometrySpan
    {
      std::vector<LinePass::Vertex> lineVertices;
      std::vector<TextPass::Vertex> textVertices;
      Math::BoxAABB bbox;
    };

    using PlacementMap = std::unordered_map<Core::Handle, Placement>;

    void rebuildMain ( const Cad::Document &document, Core::Font &font );
    void appendAddedEntity ( const Cad::Document &document, Core::Handle handle, Core::Font &font );
    void clearOverlay ();
    void rebuildVisibleDrawLists ( const Core::Camera &camera ) const;

    static GeometrySpan buildGeometry ( const Entity::Entity &entity, Core::Font &font );
    bool overlayTooLarge () const;

    static glm::vec4 colorForRenderKey ( std::uint32_t renderKey );

    std::vector<Page> m_pages;
    PlacementMap m_placements;

    std::vector<LinePass::Vertex> m_mainLineVertices;
    std::vector<TextPass::Vertex> m_mainTextVertices;
    VertexBatch<LinePass::Vertex> m_mainLineBatch;
    VertexBatch<TextPass::Vertex> m_mainTextBatch;

    std::vector<LinePass::Vertex> m_overlayLineVertices;
    std::vector<TextPass::Vertex> m_overlayTextVertices;
    VertexBatch<LinePass::Vertex> m_overlayLineBatch;
    VertexBatch<TextPass::Vertex> m_overlayTextBatch;
    std::vector<Placement> m_overlayPlacements;

    mutable VisibleDrawList m_visibleMainLines;
    mutable VisibleDrawList m_visibleMainTexts;
    mutable VisibleDrawList m_visibleOverlayLines;
    mutable VisibleDrawList m_visibleOverlayTexts;

    bool m_initialized = false;
    bool m_bootstrapped = false;
    float m_renderKeyScale = 1.0f;

    LinePass m_linePass;
    TextPass m_textPass;
  };
} // namespace Qadra::Render

#endif // QADRA_RENDER_SCENE_HPP
