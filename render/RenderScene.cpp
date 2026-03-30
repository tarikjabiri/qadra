#include "RenderScene.hpp"

#include "ArcInstanceBuilder.hpp"
#include "CircleInstanceBuilder.hpp"
#include "EllipseInstanceBuilder.hpp"
#include "LWPolylineGeometryBuilder.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace
{
  [[nodiscard]] float normalizedByte ( const std::uint32_t value, const int shift )
  {
    return static_cast<float> ( ( value >> shift ) & 0xFFu ) / 255.0f;
  }
} // namespace

namespace Qadra::Render
{
  void RenderScene::init ()
  {
    m_arcPass.init ();
    m_ellipsePass.init ();
    m_linePass.init ();
    m_textPass.init ();
    m_initialized = true;
  }

  void RenderScene::sync ( const Cad::Document &document, Core::Font &font )
  {
    if ( ! m_initialized ) return;

    m_renderKeyScale =
        document.maxRenderKey () == 0
            ? 1.0f
            : 1.0f / static_cast<float> ( static_cast<double> ( document.maxRenderKey () ) + 1.0 );

    const auto changes = document.drainChanges ();
    if ( ! m_bootstrapped )
    {
      rebuildAll ( document, font );
      return;
    }

    bool needsFullRebuild = false;
    bool structureChanged = false;

    for ( const auto &change : changes )
    {
      switch ( change.kind )
      {
        case Cad::DocumentChange::Kind::Added:
          addEntity ( document, change.handle, font );
          structureChanged = true;
          break;
        case Cad::DocumentChange::Kind::Removed:
          structureChanged = removeEntity ( change.handle ) || structureChanged;
          break;
        case Cad::DocumentChange::Kind::Modified:
          structureChanged = modifyEntity ( document, change.handle, font ) || structureChanged;
          break;
        case Cad::DocumentChange::Kind::Reset:
          needsFullRebuild = true;
          break;
      }

      if ( needsFullRebuild ) break;
    }

    if ( needsFullRebuild || ( structureChanged && shouldCompact () ) )
    {
      rebuildAll ( document, font );
      return;
    }

    refreshDirtyTextPages ();
  }

  void RenderScene::draw ( const Core::Camera &camera, const Core::Font &font ) const
  {
    const std::array<GLint, 1> firsts{ 0 };

    if ( const auto count = static_cast<GLsizei> ( m_arcBuffer.highWater () ); count > 0 )
    {
      const std::array<GLsizei, 1> counts{ count };
      m_arcPass.renderRanges ( camera, m_arcBuffer.buffer (), firsts, counts, m_renderKeyScale );
    }

    if ( const auto count = static_cast<GLsizei> ( m_ellipseBuffer.highWater () ); count > 0 )
    {
      const std::array<GLsizei, 1> counts{ count };
      m_ellipsePass.renderRanges ( camera, m_ellipseBuffer.buffer (), firsts, counts,
                                   m_renderKeyScale );
    }

    if ( const auto count = static_cast<GLsizei> ( m_lineBuffer.highWater () ); count > 0 )
    {
      const std::array<GLsizei, 1> counts{ count };
      m_linePass.renderRanges ( camera, m_lineBuffer.buffer (), firsts, counts, m_renderKeyScale );
    }

    uploadVisibleTextCommands ( camera );
    m_textPass.renderIndirect ( camera, font.texture (), font.distanceFieldRange (),
                                m_textBuffer.buffer (), m_textCommandBuffer,
                                static_cast<GLsizei> ( m_visibleTextCommands.size () ),
                                m_renderKeyScale );
  }

  void RenderScene::addEntity ( const Cad::Document &document, const Core::Handle handle,
                                Core::Font &font )
  {
    const auto *entity = document.find ( handle );
    if ( entity == nullptr ) return;

    if ( m_placements.contains ( handle ) ) static_cast<void> ( removeEntity ( handle ) );

    const GeometrySpan geometry = buildGeometry ( *entity, font );
    Placement placement;
    placement.arcSlots =
        m_arcBuffer.insert ( std::span<const ArcPass::Instance> ( geometry.arcInstances ) );
    placement.ellipseSlots = m_ellipseBuffer.insert (
        std::span<const EllipsePass::Instance> ( geometry.ellipseInstances ) );
    placement.lineSlots =
        m_lineBuffer.insert ( std::span<const LinePass::Vertex> ( geometry.lineVertices ) );
    placement.textSlots =
        m_textBuffer.insert ( std::span<const TextPass::Instance> ( geometry.textInstances ) );
    placement.bbox = entity->bbox ();

    if ( ! placement.textSlots.empty () )
      placement.textPage = appendTextPage ( placement.textSlots, placement.bbox, { handle } );

    m_placements[handle] = placement;
  }

  bool RenderScene::removeEntity ( const Core::Handle handle )
  {
    const auto it = m_placements.find ( handle );
    if ( it == m_placements.end () ) return false;

    const Placement placement = it->second;
    if ( placement.textPage != kInvalidTextPage && placement.textPage < m_textPages.size () )
    {
      auto &page = m_textPages[placement.textPage];
      page.handles.erase ( std::remove ( page.handles.begin (), page.handles.end (), handle ),
                           page.handles.end () );
      markTextPageDirty ( placement.textPage );
    }

    m_arcBuffer.erase ( placement.arcSlots );
    m_ellipseBuffer.erase ( placement.ellipseSlots );
    m_lineBuffer.erase ( placement.lineSlots );
    m_textBuffer.erase ( placement.textSlots,
                         ManagedBuffer<TextPass::Instance>::WriteMode::Immediate, false );
    m_deadTextInstances += placement.textSlots.count;

    m_placements.erase ( it );
    return true;
  }

  bool RenderScene::modifyEntity ( const Cad::Document &document, const Core::Handle handle,
                                   Core::Font &font )
  {
    const auto placementIt = m_placements.find ( handle );
    if ( placementIt == m_placements.end () )
    {
      addEntity ( document, handle, font );
      return true;
    }

    const auto *entity = document.find ( handle );
    if ( entity == nullptr ) return removeEntity ( handle );

    const Placement &oldPlacement = placementIt->second;
    const GeometrySpan geometry = buildGeometry ( *entity, font );

    const bool sameShape = geometry.arcInstances.size () == oldPlacement.arcSlots.count &&
                           geometry.ellipseInstances.size () == oldPlacement.ellipseSlots.count &&
                           geometry.lineVertices.size () == oldPlacement.lineSlots.count &&
                           geometry.textInstances.size () == oldPlacement.textSlots.count;

    if ( sameShape )
    {
      m_arcBuffer.update ( oldPlacement.arcSlots,
                           std::span<const ArcPass::Instance> ( geometry.arcInstances ) );
      m_ellipseBuffer.update ( oldPlacement.ellipseSlots, std::span<const EllipsePass::Instance> (
                                                              geometry.ellipseInstances ) );
      m_lineBuffer.update ( oldPlacement.lineSlots,
                            std::span<const LinePass::Vertex> ( geometry.lineVertices ) );
      m_textBuffer.update ( oldPlacement.textSlots,
                            std::span<const TextPass::Instance> ( geometry.textInstances ) );
      placementIt->second.bbox = entity->bbox ();
      if ( placementIt->second.textPage != kInvalidTextPage )
        markTextPageDirty ( placementIt->second.textPage );
      return false;
    }

    static_cast<void> ( removeEntity ( handle ) );
    addEntity ( document, handle, font );
    return true;
  }

  void RenderScene::rebuildAll ( const Cad::Document &document, Core::Font &font )
  {
    m_arcBuffer.clear ();
    m_ellipseBuffer.clear ();
    m_lineBuffer.clear ();
    m_textBuffer.clear ();
    m_placements.clear ();
    m_textPages.clear ();
    m_deadTextInstances = 0;

    SlotAllocator::Range currentTextPageSlots;
    Math::BoxAABB currentTextPageBBox;
    std::vector<Core::Handle> currentTextPageHandles;

    auto flushCurrentTextPage = [&] ()
    {
      if ( currentTextPageSlots.empty () || currentTextPageHandles.empty () ) return;

      const std::size_t pageIndex =
          appendTextPage ( currentTextPageSlots, currentTextPageBBox, currentTextPageHandles );
      for ( const Core::Handle textHandle : currentTextPageHandles )
      {
        auto placementIt = m_placements.find ( textHandle );
        if ( placementIt != m_placements.end () ) placementIt->second.textPage = pageIndex;
      }

      currentTextPageSlots = {};
      currentTextPageBBox = Math::BoxAABB ();
      currentTextPageHandles.clear ();
    };

    for ( const Core::Handle handle : document.drawOrder () )
    {
      const auto *entity = document.find ( handle );
      if ( entity == nullptr ) continue;

      const GeometrySpan geometry = buildGeometry ( *entity, font );
      Placement placement;
      placement.arcSlots =
          m_arcBuffer.insert ( std::span<const ArcPass::Instance> ( geometry.arcInstances ),
                               ManagedBuffer<ArcPass::Instance>::WriteMode::Deferred );
      placement.ellipseSlots = m_ellipseBuffer.insert (
          std::span<const EllipsePass::Instance> ( geometry.ellipseInstances ),
          ManagedBuffer<EllipsePass::Instance>::WriteMode::Deferred );
      placement.lineSlots =
          m_lineBuffer.insert ( std::span<const LinePass::Vertex> ( geometry.lineVertices ),
                                ManagedBuffer<LinePass::Vertex>::WriteMode::Deferred );
      placement.textSlots =
          m_textBuffer.insert ( std::span<const TextPass::Instance> ( geometry.textInstances ),
                                ManagedBuffer<TextPass::Instance>::WriteMode::Deferred );
      placement.bbox = entity->bbox ();

      m_placements[handle] = placement;

      if ( ! placement.textSlots.empty () )
      {
        const bool pageWouldOverflow =
            ! currentTextPageSlots.empty () &&
            currentTextPageSlots.count + placement.textSlots.count > kMaxTextInstancesPerPage;
        if ( pageWouldOverflow ) flushCurrentTextPage ();

        if ( currentTextPageSlots.empty () )
        {
          currentTextPageSlots = placement.textSlots;
          currentTextPageBBox = placement.bbox;
        }
        else
        {
          currentTextPageSlots.count += placement.textSlots.count;
          currentTextPageBBox.merge ( placement.bbox );
        }

        currentTextPageHandles.push_back ( handle );
      }
    }

    flushCurrentTextPage ();

    m_arcBuffer.uploadAll ();
    m_ellipseBuffer.uploadAll ();
    m_lineBuffer.uploadAll ();
    m_textBuffer.uploadAll ();

    m_bootstrapped = true;
    refreshDirtyTextPages ();
  }

  void RenderScene::uploadVisibleTextCommands ( const Core::Camera &camera ) const
  {
    m_visibleTextCommands.clear ();

    const Math::BoxAABB viewport = camera.viewportBox ();
    for ( const TextPage &page : m_textPages )
    {
      if ( page.instanceRange.empty () || page.handles.empty () ) continue;
      if ( ! page.bbox.intersects ( viewport ) ) continue;

      TextPass::DrawCommand command;
      command.count = 6;
      command.instanceCount = page.instanceRange.count;
      command.first = 0;
      command.baseInstance = page.instanceRange.first;
      m_visibleTextCommands.push_back ( command );
    }

    if ( ! m_visibleTextCommands.empty () )
      m_textCommandBuffer.allocate (
          std::span<const TextPass::DrawCommand> ( m_visibleTextCommands ),
          GL::Buffer::Usage::DynamicDraw );
  }

  void RenderScene::refreshDirtyTextPages ()
  {
    for ( std::size_t pageIndex = 0; pageIndex < m_textPages.size (); ++pageIndex )
    {
      TextPage &page = m_textPages[pageIndex];
      if ( ! page.bboxDirty ) continue;

      std::vector<Core::Handle> validHandles;
      validHandles.reserve ( page.handles.size () );

      Math::BoxAABB bbox;
      bool hasBounds = false;

      for ( const Core::Handle handle : page.handles )
      {
        const auto placementIt = m_placements.find ( handle );
        if ( placementIt == m_placements.end () ) continue;
        if ( placementIt->second.textPage != pageIndex ) continue;
        if ( placementIt->second.textSlots.empty () ) continue;

        validHandles.push_back ( handle );
        if ( ! hasBounds )
        {
          bbox = placementIt->second.bbox;
          hasBounds = true;
        }
        else
          bbox.merge ( placementIt->second.bbox );
      }

      page.handles = std::move ( validHandles );
      if ( hasBounds ) page.bbox = bbox;
      page.bboxDirty = false;
    }
  }

  void RenderScene::markTextPageDirty ( const std::size_t pageIndex )
  {
    if ( pageIndex == kInvalidTextPage || pageIndex >= m_textPages.size () ) return;
    m_textPages[pageIndex].bboxDirty = true;
  }

  std::size_t RenderScene::appendTextPage ( const SlotAllocator::Range instanceRange,
                                            const Math::BoxAABB bbox,
                                            std::vector<Core::Handle> handles )
  {
    if ( instanceRange.empty () || handles.empty () ) return kInvalidTextPage;

    TextPage page;
    page.instanceRange = instanceRange;
    page.bbox = bbox;
    page.handles = std::move ( handles );
    m_textPages.push_back ( std::move ( page ) );
    return m_textPages.size () - 1;
  }

  RenderScene::GeometrySpan RenderScene::buildGeometry ( const Entity::Entity &entity,
                                                         Core::Font &font )
  {
    GeometrySpan geometry;

    const glm::vec4 color = colorForRenderKey ( entity.renderKey () );
    const std::uint32_t renderKey = entity.renderKey ();

    switch ( entity.type () )
    {
      case Entity::EntityType::Arc:
      {
        const auto &arc = static_cast<const Entity::Arc &> ( entity );
        geometry.arcInstances.push_back ( buildArcInstance ( arc.curve (), color, renderKey ) );
        break;
      }
      case Entity::EntityType::LWPolyline:
      {
        const auto &lwPolyline = static_cast<const Entity::LWPolyline &> ( entity );
        const LWPolylineGeometry polylineGeometry =
            buildLWPolylineGeometry ( lwPolyline, color, renderKey );
        geometry.arcInstances.insert ( geometry.arcInstances.end (),
                                       polylineGeometry.arcInstances.begin (),
                                       polylineGeometry.arcInstances.end () );
        geometry.lineVertices.insert ( geometry.lineVertices.end (),
                                       polylineGeometry.lineVertices.begin (),
                                       polylineGeometry.lineVertices.end () );
        for ( auto &vertex : geometry.lineVertices ) vertex.flags = 1u;
        break;
      }
      case Entity::EntityType::Circle:
      {
        const auto &circle = static_cast<const Entity::Circle &> ( entity );
        geometry.ellipseInstances.push_back (
            buildCircleInstance ( circle.curve (), color, renderKey ) );
        break;
      }
      case Entity::EntityType::Ellipse:
      {
        const auto &ellipse = static_cast<const Entity::Ellipse &> ( entity );
        geometry.ellipseInstances.push_back (
            buildEllipseInstance ( ellipse.curve (), color, renderKey ) );
        break;
      }
      case Entity::EntityType::Line:
      {
        const auto &line = static_cast<const Entity::Line &> ( entity );
        geometry.lineVertices.push_back ( { line.start (), color, renderKey, 1u } );
        geometry.lineVertices.push_back ( { line.end (), color, renderKey, 1u } );
        break;
      }
      case Entity::EntityType::Text:
      {
        const auto &text = static_cast<const Entity::Text &> ( entity );
        const double scale = text.height () / font.unitsPerEm ();
        const float cosAngle = static_cast<float> ( std::cos ( text.rotation () ) );
        const float sinAngle = static_cast<float> ( std::sin ( text.rotation () ) );
        const auto packedColor = std::array<std::uint8_t, 4>{
            packUnorm8 ( color.r ),
            packUnorm8 ( color.g ),
            packUnorm8 ( color.b ),
            packUnorm8 ( color.a ),
        };
        const auto packedRotation = std::array<std::int16_t, 2>{
            packSnorm16 ( cosAngle ),
            packSnorm16 ( sinAngle ),
        };

        glm::dvec2 cursor ( 0.0 );
        glm::dvec2 textBoxMin ( 0.0 );
        glm::dvec2 textBoxMax ( 0.0 );
        bool hasTextBox = false;
        for ( const auto &shapedGlyph : text.layout ().glyphs )
        {
          const auto &glyph = font.glyph ( shapedGlyph.glyphId );
          if ( ! glyph.empty )
          {
            const glm::dvec2 offset = ( cursor + shapedGlyph.offset ) * scale;
            const glm::dvec2 qMin = offset + glm::dvec2 ( glyph.quadMin ) * text.height ();
            const glm::dvec2 qMax = offset + glm::dvec2 ( glyph.quadMax ) * text.height ();

            if ( ! hasTextBox )
            {
              textBoxMin = qMin;
              textBoxMax = qMax;
              hasTextBox = true;
            }
            else
            {
              textBoxMin = glm::min ( textBoxMin, qMin );
              textBoxMax = glm::max ( textBoxMax, qMax );
            }
          }

          cursor += shapedGlyph.advance;
        }

        if ( ! hasTextBox ) break;

        cursor = glm::dvec2 ( 0.0 );
        bool markerAnchorAssigned = false;
        for ( const auto &shapedGlyph : text.layout ().glyphs )
        {
          const auto &glyph = font.glyph ( shapedGlyph.glyphId );
          if ( ! glyph.empty )
          {
            const glm::dvec2 offset = ( cursor + shapedGlyph.offset ) * scale;
            const glm::dvec2 qMin = offset + glm::dvec2 ( glyph.quadMin ) * text.height ();
            const glm::dvec2 qMax = offset + glm::dvec2 ( glyph.quadMax ) * text.height ();

            std::uint32_t flags = TextPass::kFlagAlive;
            if ( ! markerAnchorAssigned )
            {
              flags |= TextPass::kFlagMarkerAnchor;
              markerAnchorAssigned = true;
            }

            TextPass::Instance instance;
            instance.textOriginWorld = glm::vec2 ( text.position () );
            instance.quadMinLocal = glm::vec2 ( qMin );
            instance.quadMaxLocal = glm::vec2 ( qMax );
            instance.textBoxMinLocal = glm::vec2 ( textBoxMin );
            instance.textBoxMaxLocal = glm::vec2 ( textBoxMax );
            instance.uvMin = { packUnorm16 ( glyph.uvMin.x ), packUnorm16 ( glyph.uvMin.y ) };
            instance.uvMax = { packUnorm16 ( glyph.uvMax.x ), packUnorm16 ( glyph.uvMax.y ) };
            instance.rotation = packedRotation;
            instance.color = packedColor;
            instance.renderKey = renderKey;
            instance.flags = flags;
            geometry.textInstances.push_back ( instance );
          }

          cursor += shapedGlyph.advance;
        }
        break;
      }
    }

    return geometry;
  }

  bool RenderScene::shouldCompact () const
  {
    constexpr float kFragmentationThreshold = 0.35f;

    return m_arcBuffer.fragmentation () > kFragmentationThreshold ||
           m_ellipseBuffer.fragmentation () > kFragmentationThreshold ||
           m_lineBuffer.fragmentation () > kFragmentationThreshold ||
           ( m_textBuffer.highWater () > 0 &&
             static_cast<float> ( m_deadTextInstances ) /
                     static_cast<float> ( m_textBuffer.highWater () ) >
                 kFragmentationThreshold );
  }

  glm::vec4 RenderScene::colorForRenderKey ( const std::uint32_t renderKey )
  {
    const std::uint32_t mixed = renderKey * 2654435761u;
    return {
        0.2f + 0.6f * normalizedByte ( mixed, 0 ),
        0.2f + 0.6f * normalizedByte ( mixed, 8 ),
        0.2f + 0.6f * normalizedByte ( mixed, 16 ),
        1.0f,
    };
  }

  std::uint16_t RenderScene::packUnorm16 ( const float value )
  {
    const float clamped = std::clamp ( value, 0.0f, 1.0f );
    return static_cast<std::uint16_t> ( std::lround ( clamped * 65535.0f ) );
  }

  std::int16_t RenderScene::packSnorm16 ( const float value )
  {
    const float clamped = std::clamp ( value, -1.0f, 1.0f );
    if ( clamped <= -1.0f ) return std::numeric_limits<std::int16_t>::min ();
    return static_cast<std::int16_t> ( std::lround ( clamped * 32767.0f ) );
  }

  std::uint8_t RenderScene::packUnorm8 ( const float value )
  {
    const float clamped = std::clamp ( value, 0.0f, 1.0f );
    return static_cast<std::uint8_t> ( std::lround ( clamped * 255.0f ) );
  }
} // namespace Qadra::Render
