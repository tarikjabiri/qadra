#ifndef QADRA_RENDER_SCENE_HPP
#define QADRA_RENDER_SCENE_HPP

#include "Document.hpp"
#include "LinePass.hpp"
#include "TextPass.hpp"
#include "VertexBatch.hpp"
#include "VisibleDrawList.hpp"
#include "gl/Buffer.hpp"

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
    static constexpr std::size_t kMaxTextInstancesPerPage = 4096;
    static constexpr std::size_t kMaxOverlayEntities = 512;
    static constexpr std::size_t kMaxOverlayLineVertices = 16384;
    static constexpr std::size_t kMaxOverlayTextInstances = 32768;

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
      GLuint textInstanceFirst = 0;
      GLuint textInstanceCount = 0;
    };

    struct Placement
    {
      StorageLocation storage = StorageLocation::Main;
      Math::BoxAABB bbox;
      GLint lineFirst = 0;
      GLsizei lineCount = 0;
      GLuint textInstanceFirst = 0;
      GLuint textInstanceCount = 0;
    };

    struct GeometrySpan
    {
      std::vector<LinePass::Vertex> lineVertices;
      std::vector<TextPass::Instance> textInstances;
      Math::BoxAABB bbox;
    };

    using PlacementMap = std::unordered_map<Core::Handle, Placement>;

    void rebuildMain ( const Cad::Document &document, Core::Font &font );
    void appendAddedEntity ( const Cad::Document &document, Core::Handle handle, Core::Font &font );
    void clearOverlay ();
    void rebuildVisibleDrawLists ( const Core::Camera &camera ) const;
    void uploadVisibleTextCommands () const;

    static GeometrySpan buildGeometry ( const Entity::Entity &entity, Core::Font &font );
    bool overlayTooLarge () const;

    static glm::vec4 colorForRenderKey ( std::uint32_t renderKey );
    static std::uint16_t packUnorm16 ( float value );
    static std::int16_t packSnorm16 ( float value );
    static std::uint8_t packUnorm8 ( float value );

    std::vector<Page> m_pages;
    PlacementMap m_placements;

    std::vector<LinePass::Vertex> m_mainLineVertices;
    std::vector<TextPass::Instance> m_mainTextInstances;
    VertexBatch<LinePass::Vertex> m_mainLineBatch;
    VertexBatch<TextPass::Instance> m_mainTextBatch;

    std::vector<LinePass::Vertex> m_overlayLineVertices;
    std::vector<TextPass::Instance> m_overlayTextInstances;
    VertexBatch<LinePass::Vertex> m_overlayLineBatch;
    VertexBatch<TextPass::Instance> m_overlayTextBatch;
    std::vector<Placement> m_overlayPlacements;

    mutable VisibleDrawList m_visibleMainLines;
    mutable VisibleDrawList m_visibleOverlayLines;
    mutable std::vector<TextPass::DrawCommand> m_visibleMainTextCommands;
    mutable std::vector<TextPass::DrawCommand> m_visibleOverlayTextCommands;
    mutable GL::Buffer m_mainTextCommandBuffer{ GL::Buffer::Target::DrawIndirectBuffer };
    mutable GL::Buffer m_overlayTextCommandBuffer{ GL::Buffer::Target::DrawIndirectBuffer };

    bool m_initialized = false;
    bool m_bootstrapped = false;
    float m_renderKeyScale = 1.0f;

    LinePass m_linePass;
    TextPass m_textPass;
  };
} // namespace Qadra::Render

#endif // QADRA_RENDER_SCENE_HPP
