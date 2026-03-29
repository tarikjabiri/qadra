#ifndef QADRA_RENDER_SCENE_HPP
#define QADRA_RENDER_SCENE_HPP

#include "ArcPass.hpp"
#include "Document.hpp"
#include "EllipsePass.hpp"
#include "LinePass.hpp"
#include "ManagedBuffer.hpp"
#include "TextPass.hpp"
#include "gl/Buffer.hpp"

#include <limits>
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
    static constexpr std::size_t kInvalidTextPage = std::numeric_limits<std::size_t>::max ();
    static constexpr std::size_t kMaxTextInstancesPerPage = 4096;

    struct Placement
    {
      SlotAllocator::Range arcSlots;
      SlotAllocator::Range ellipseSlots;
      SlotAllocator::Range lineSlots;
      SlotAllocator::Range textSlots;
      Math::BoxAABB bbox;
      std::size_t textPage = kInvalidTextPage;
    };

    struct TextPage
    {
      SlotAllocator::Range instanceRange;
      Math::BoxAABB bbox;
      std::vector<Core::Handle> handles;
      bool bboxDirty = false;
    };

    struct GeometrySpan
    {
      std::vector<ArcPass::Instance> arcInstances;
      std::vector<EllipsePass::Instance> ellipseInstances;
      std::vector<LinePass::Vertex> lineVertices;
      std::vector<TextPass::Instance> textInstances;
    };

    void addEntity ( const Cad::Document &document, Core::Handle handle, Core::Font &font );
    [[nodiscard]] bool removeEntity ( Core::Handle handle );
    [[nodiscard]] bool modifyEntity ( const Cad::Document &document, Core::Handle handle,
                                      Core::Font &font );
    void rebuildAll ( const Cad::Document &document, Core::Font &font );
    void uploadVisibleTextCommands ( const Core::Camera &camera ) const;
    void refreshDirtyTextPages ();
    void markTextPageDirty ( std::size_t pageIndex );
    std::size_t appendTextPage ( SlotAllocator::Range instanceRange, Math::BoxAABB bbox,
                                 std::vector<Core::Handle> handles );

    [[nodiscard]] static GeometrySpan buildGeometry ( const Entity::Entity &entity,
                                                      Core::Font &font );
    [[nodiscard]] bool shouldCompact () const;

    [[nodiscard]] static glm::vec4 colorForRenderKey ( std::uint32_t renderKey );
    [[nodiscard]] static std::uint16_t packUnorm16 ( float value );
    [[nodiscard]] static std::int16_t packSnorm16 ( float value );
    [[nodiscard]] static std::uint8_t packUnorm8 ( float value );

    ManagedBuffer<ArcPass::Instance> m_arcBuffer;
    ManagedBuffer<EllipsePass::Instance> m_ellipseBuffer;
    ManagedBuffer<LinePass::Vertex> m_lineBuffer;
    ManagedBuffer<TextPass::Instance> m_textBuffer;

    std::unordered_map<Core::Handle, Placement> m_placements;
    std::vector<TextPage> m_textPages;

    mutable std::vector<TextPass::DrawCommand> m_visibleTextCommands;
    mutable GL::Buffer m_textCommandBuffer{ GL::Buffer::Target::DrawIndirectBuffer };

    std::size_t m_deadTextInstances = 0;
    bool m_initialized = false;
    bool m_bootstrapped = false;
    float m_renderKeyScale = 1.0f;

    ArcPass m_arcPass;
    EllipsePass m_ellipsePass;
    LinePass m_linePass;
    TextPass m_textPass;
  };
} // namespace Qadra::Render

#endif // QADRA_RENDER_SCENE_HPP
