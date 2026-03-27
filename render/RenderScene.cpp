#include "RenderScene.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace
{
  float normalizedByte ( const std::uint32_t value, const int shift )
  {
    return static_cast<float> ( ( value >> shift ) & 0xFFu ) / 255.0f;
  }
} // namespace

namespace Qadra::Render
{
  void RenderScene::init ()
  {
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
      rebuildMain ( document, font );
      return;
    }

    bool requiresRebuild = false;
    for ( const auto &change : changes )
    {
      switch ( change.kind )
      {
        case Cad::DocumentChange::Kind::Added:
          appendAddedEntity ( document, change.handle, font );
          break;
        case Cad::DocumentChange::Kind::Removed:
        case Cad::DocumentChange::Kind::Reset:
          requiresRebuild = true;
          break;
      }

      if ( requiresRebuild ) break;
    }

    if ( requiresRebuild || overlayTooLarge () ) rebuildMain ( document, font );
  }

  void RenderScene::draw ( const Core::Camera &camera, const Core::Font &font ) const
  {
    rebuildVisibleDrawLists ( camera );
    uploadVisibleTextCommands ();

    m_linePass.renderRanges ( camera, m_mainLineBatch.buffer (), m_visibleMainLines.firsts (),
                              m_visibleMainLines.counts (), m_renderKeyScale );
    m_linePass.renderRanges ( camera, m_overlayLineBatch.buffer (), m_visibleOverlayLines.firsts (),
                              m_visibleOverlayLines.counts (), m_renderKeyScale );

    m_textPass.renderIndirect ( camera, font.texture (), font.distanceFieldRange (),
                                m_mainTextBatch.buffer (), m_mainTextCommandBuffer,
                                static_cast<GLsizei> ( m_visibleMainTextCommands.size () ),
                                m_renderKeyScale );
    m_textPass.renderIndirect ( camera, font.texture (), font.distanceFieldRange (),
                                m_overlayTextBatch.buffer (), m_overlayTextCommandBuffer,
                                static_cast<GLsizei> ( m_visibleOverlayTextCommands.size () ),
                                m_renderKeyScale );
  }

  void RenderScene::rebuildMain ( const Cad::Document &document, Core::Font &font )
  {
    m_pages.clear ();
    m_placements.clear ();
    m_mainLineVertices.clear ();
    m_mainTextInstances.clear ();
    clearOverlay ();

    const auto &order = document.drawOrder ();

    Page currentPage;
    bool hasCurrentPage = false;
    std::size_t entitiesInPage = 0;
    std::size_t lineVerticesInPage = 0;
    std::size_t textInstancesInPage = 0;

    auto beginPage = [&] ()
    {
      currentPage = Page{
          .bbox = Math::BoxAABB (),
          .lineFirst = static_cast<GLint> ( m_mainLineVertices.size () ),
          .lineCount = 0,
          .textInstanceFirst = static_cast<GLuint> ( m_mainTextInstances.size () ),
          .textInstanceCount = 0,
      };
      entitiesInPage = 0;
      lineVerticesInPage = 0;
      textInstancesInPage = 0;
      hasCurrentPage = true;
    };

    auto finishPage = [&] ()
    {
      if ( ! hasCurrentPage ) return;

      currentPage.lineCount = static_cast<GLsizei> ( lineVerticesInPage );
      currentPage.textInstanceCount = static_cast<GLuint> ( textInstancesInPage );
      m_pages.push_back ( currentPage );
      hasCurrentPage = false;
    };

    for ( const auto handle : order )
    {
      const auto *entity = document.find ( handle );
      if ( ! entity ) continue;

      GeometrySpan geometry = buildGeometry ( *entity, font );

      const bool pageWouldOverflow =
          hasCurrentPage &&
          ( entitiesInPage >= kMaxEntitiesPerPage ||
            lineVerticesInPage + geometry.lineVertices.size () > kMaxLineVerticesPerPage ||
            textInstancesInPage + geometry.textInstances.size () > kMaxTextInstancesPerPage );

      if ( ! hasCurrentPage || pageWouldOverflow )
      {
        finishPage ();
        beginPage ();
      }

      const GLint lineFirst = static_cast<GLint> ( m_mainLineVertices.size () );
      const GLuint textInstanceFirst = static_cast<GLuint> ( m_mainTextInstances.size () );

      m_mainLineVertices.insert ( m_mainLineVertices.end (), geometry.lineVertices.begin (),
                                  geometry.lineVertices.end () );
      m_mainTextInstances.insert ( m_mainTextInstances.end (), geometry.textInstances.begin (),
                                   geometry.textInstances.end () );

      currentPage.bbox.merge ( geometry.bbox );
      lineVerticesInPage += geometry.lineVertices.size ();
      textInstancesInPage += geometry.textInstances.size ();
      ++entitiesInPage;

      m_placements[handle] = Placement{
          .storage = StorageLocation::Main,
          .bbox = geometry.bbox,
          .lineFirst = lineFirst,
          .lineCount = static_cast<GLsizei> ( geometry.lineVertices.size () ),
          .textInstanceFirst = textInstanceFirst,
          .textInstanceCount = static_cast<GLuint> ( geometry.textInstances.size () ),
      };
    }

    finishPage ();

    m_mainLineBatch.upload ( std::span<const LinePass::Vertex> ( m_mainLineVertices ),
                             GL::Buffer::Usage::StaticDraw );
    m_mainTextBatch.upload ( std::span<const TextPass::Instance> ( m_mainTextInstances ),
                             GL::Buffer::Usage::StaticDraw );
    m_bootstrapped = true;
  }

  void RenderScene::appendAddedEntity ( const Cad::Document &document, const Core::Handle handle,
                                        Core::Font &font )
  {
    const auto *entity = document.find ( handle );
    if ( ! entity ) return;

    GeometrySpan geometry = buildGeometry ( *entity, font );

    const GLint lineFirst = static_cast<GLint> ( m_overlayLineVertices.size () );
    const GLuint textInstanceFirst = static_cast<GLuint> ( m_overlayTextInstances.size () );

    m_overlayLineVertices.insert ( m_overlayLineVertices.end (), geometry.lineVertices.begin (),
                                   geometry.lineVertices.end () );
    m_overlayTextInstances.insert ( m_overlayTextInstances.end (), geometry.textInstances.begin (),
                                    geometry.textInstances.end () );

    m_overlayLineBatch.upload ( std::span<const LinePass::Vertex> ( m_overlayLineVertices ),
                                GL::Buffer::Usage::DynamicDraw );
    m_overlayTextBatch.upload ( std::span<const TextPass::Instance> ( m_overlayTextInstances ),
                                GL::Buffer::Usage::DynamicDraw );

    const Placement placement{
        .storage = StorageLocation::Overlay,
        .bbox = geometry.bbox,
        .lineFirst = lineFirst,
        .lineCount = static_cast<GLsizei> ( geometry.lineVertices.size () ),
        .textInstanceFirst = textInstanceFirst,
        .textInstanceCount = static_cast<GLuint> ( geometry.textInstances.size () ),
    };

    m_overlayPlacements.push_back ( placement );
    m_placements[handle] = placement;
  }

  void RenderScene::clearOverlay ()
  {
    m_overlayLineVertices.clear ();
    m_overlayTextInstances.clear ();
    m_overlayPlacements.clear ();
    m_overlayLineBatch.upload ( std::span<const LinePass::Vertex> ( m_overlayLineVertices ),
                                GL::Buffer::Usage::DynamicDraw );
    m_overlayTextBatch.upload ( std::span<const TextPass::Instance> ( m_overlayTextInstances ),
                                GL::Buffer::Usage::DynamicDraw );
  }

  void RenderScene::rebuildVisibleDrawLists ( const Core::Camera &camera ) const
  {
    m_visibleMainLines.clear ();
    m_visibleOverlayLines.clear ();
    m_visibleMainTextCommands.clear ();
    m_visibleOverlayTextCommands.clear ();

    const Math::BoxAABB viewport = camera.viewportBox ();

    for ( const auto &page : m_pages )
    {
      if ( ! page.bbox.intersects ( viewport ) ) continue;

      m_visibleMainLines.append ( page.lineFirst, page.lineCount );
      if ( page.textInstanceCount == 0 ) continue;

      m_visibleMainTextCommands.push_back ( TextPass::DrawCommand{
          .count = 6,
          .instanceCount = page.textInstanceCount,
          .first = 0,
          .baseInstance = page.textInstanceFirst,
      } );
    }

    for ( const auto &placement : m_overlayPlacements )
    {
      if ( ! placement.bbox.intersects ( viewport ) ) continue;

      m_visibleOverlayLines.append ( placement.lineFirst, placement.lineCount );
      if ( placement.textInstanceCount == 0 ) continue;

      m_visibleOverlayTextCommands.push_back ( TextPass::DrawCommand{
          .count = 6,
          .instanceCount = placement.textInstanceCount,
          .first = 0,
          .baseInstance = placement.textInstanceFirst,
      } );
    }
  }

  void RenderScene::uploadVisibleTextCommands () const
  {
    m_mainTextCommandBuffer.allocate (
        std::span<const TextPass::DrawCommand> ( m_visibleMainTextCommands ),
        GL::Buffer::Usage::DynamicDraw );
    m_overlayTextCommandBuffer.allocate (
        std::span<const TextPass::DrawCommand> ( m_visibleOverlayTextCommands ),
        GL::Buffer::Usage::DynamicDraw );
  }

  RenderScene::GeometrySpan RenderScene::buildGeometry ( const Entity::Entity &entity,
                                                         Core::Font &font )
  {
    GeometrySpan geometry;
    geometry.bbox = entity.bbox ();

    const glm::vec4 color = colorForRenderKey ( entity.renderKey () );
    const auto renderKey = entity.renderKey ();

    switch ( entity.type () )
    {
      case Entity::EntityType::Line:
      {
        const auto &line = static_cast<const Entity::Line &> ( entity );
        geometry.lineVertices.push_back ( { line.start (), color, renderKey } );
        geometry.lineVertices.push_back ( { line.end (), color, renderKey } );
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
        for ( const auto &shapedGlyph : text.layout ().glyphs )
        {
          const auto &glyph = font.glyph ( shapedGlyph.glyphId );
          if ( ! glyph.empty )
          {
            const glm::dvec2 offset = ( cursor + shapedGlyph.offset ) * scale;
            const glm::dvec2 qMin = offset + glm::dvec2 ( glyph.quadMin ) * text.height ();
            const glm::dvec2 qMax = offset + glm::dvec2 ( glyph.quadMax ) * text.height ();

            geometry.textInstances.push_back ( TextPass::Instance{
                .textOriginWorld = glm::vec2 ( text.position () ),
                .quadMinLocal = glm::vec2 ( qMin ),
                .quadMaxLocal = glm::vec2 ( qMax ),
                .uvMin = { packUnorm16 ( glyph.uvMin.x ), packUnorm16 ( glyph.uvMin.y ) },
                .uvMax = { packUnorm16 ( glyph.uvMax.x ), packUnorm16 ( glyph.uvMax.y ) },
                .rotation = packedRotation,
                .color = packedColor,
                .renderKey = renderKey,
            } );
          }

          cursor += shapedGlyph.advance;
        }
        break;
      }
    }

    return geometry;
  }

  bool RenderScene::overlayTooLarge () const
  {
    return m_overlayPlacements.size () > kMaxOverlayEntities ||
           m_overlayLineVertices.size () > kMaxOverlayLineVertices ||
           m_overlayTextInstances.size () > kMaxOverlayTextInstances;
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
