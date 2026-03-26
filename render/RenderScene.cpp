#include "RenderScene.hpp"

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

    m_linePass.renderRanges ( camera, m_mainLineBatch.buffer (), m_visibleMainLines.firsts (),
                              m_visibleMainLines.counts (), m_renderKeyScale );
    m_linePass.renderRanges ( camera, m_overlayLineBatch.buffer (), m_visibleOverlayLines.firsts (),
                              m_visibleOverlayLines.counts (), m_renderKeyScale );

    m_textPass.renderRanges ( camera, font.texture (), font.distanceFieldRange (),
                              m_mainTextBatch.buffer (), m_visibleMainTexts.firsts (),
                              m_visibleMainTexts.counts (), m_renderKeyScale );
    m_textPass.renderRanges ( camera, font.texture (), font.distanceFieldRange (),
                              m_overlayTextBatch.buffer (), m_visibleOverlayTexts.firsts (),
                              m_visibleOverlayTexts.counts (), m_renderKeyScale );
  }

  void RenderScene::rebuildMain ( const Cad::Document &document, Core::Font &font )
  {
    m_pages.clear ();
    m_placements.clear ();
    m_mainLineVertices.clear ();
    m_mainTextVertices.clear ();
    clearOverlay ();

    const auto &order = document.drawOrder ();

    Page currentPage;
    bool hasCurrentPage = false;
    std::size_t entitiesInPage = 0;
    std::size_t lineVerticesInPage = 0;
    std::size_t textVerticesInPage = 0;

    auto beginPage = [&] ()
    {
      currentPage = Page{
          .bbox = Math::BoxAABB (),
          .lineFirst = static_cast<GLint> ( m_mainLineVertices.size () ),
          .lineCount = 0,
          .textFirst = static_cast<GLint> ( m_mainTextVertices.size () ),
          .textCount = 0,
      };
      entitiesInPage = 0;
      lineVerticesInPage = 0;
      textVerticesInPage = 0;
      hasCurrentPage = true;
    };

    auto finishPage = [&] ()
    {
      if ( ! hasCurrentPage ) return;

      currentPage.lineCount = static_cast<GLsizei> ( lineVerticesInPage );
      currentPage.textCount = static_cast<GLsizei> ( textVerticesInPage );
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
            textVerticesInPage + geometry.textVertices.size () > kMaxTextVerticesPerPage );

      if ( ! hasCurrentPage || pageWouldOverflow )
      {
        finishPage ();
        beginPage ();
      }

      const GLint lineFirst = static_cast<GLint> ( m_mainLineVertices.size () );
      const GLint textFirst = static_cast<GLint> ( m_mainTextVertices.size () );

      m_mainLineVertices.insert ( m_mainLineVertices.end (), geometry.lineVertices.begin (),
                                  geometry.lineVertices.end () );
      m_mainTextVertices.insert ( m_mainTextVertices.end (), geometry.textVertices.begin (),
                                  geometry.textVertices.end () );

      currentPage.bbox.merge ( geometry.bbox );
      lineVerticesInPage += geometry.lineVertices.size ();
      textVerticesInPage += geometry.textVertices.size ();
      ++entitiesInPage;

      m_placements[handle] = Placement{
          .storage = StorageLocation::Main,
          .bbox = geometry.bbox,
          .lineFirst = lineFirst,
          .lineCount = static_cast<GLsizei> ( geometry.lineVertices.size () ),
          .textFirst = textFirst,
          .textCount = static_cast<GLsizei> ( geometry.textVertices.size () ),
      };
    }

    finishPage ();

    m_mainLineBatch.upload ( std::span<const LinePass::Vertex> ( m_mainLineVertices ),
                             GL::Buffer::Usage::StaticDraw );
    m_mainTextBatch.upload ( std::span<const TextPass::Vertex> ( m_mainTextVertices ),
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
    const GLint textFirst = static_cast<GLint> ( m_overlayTextVertices.size () );

    m_overlayLineVertices.insert ( m_overlayLineVertices.end (), geometry.lineVertices.begin (),
                                   geometry.lineVertices.end () );
    m_overlayTextVertices.insert ( m_overlayTextVertices.end (), geometry.textVertices.begin (),
                                   geometry.textVertices.end () );

    m_overlayLineBatch.upload ( std::span<const LinePass::Vertex> ( m_overlayLineVertices ),
                                GL::Buffer::Usage::DynamicDraw );
    m_overlayTextBatch.upload ( std::span<const TextPass::Vertex> ( m_overlayTextVertices ),
                                GL::Buffer::Usage::DynamicDraw );

    const Placement placement{
        .storage = StorageLocation::Overlay,
        .bbox = geometry.bbox,
        .lineFirst = lineFirst,
        .lineCount = static_cast<GLsizei> ( geometry.lineVertices.size () ),
        .textFirst = textFirst,
        .textCount = static_cast<GLsizei> ( geometry.textVertices.size () ),
    };

    m_overlayPlacements.push_back ( placement );
    m_placements[handle] = placement;
  }

  void RenderScene::clearOverlay ()
  {
    m_overlayLineVertices.clear ();
    m_overlayTextVertices.clear ();
    m_overlayPlacements.clear ();
    m_overlayLineBatch.upload ( std::span<const LinePass::Vertex> ( m_overlayLineVertices ),
                                GL::Buffer::Usage::DynamicDraw );
    m_overlayTextBatch.upload ( std::span<const TextPass::Vertex> ( m_overlayTextVertices ),
                                GL::Buffer::Usage::DynamicDraw );
  }

  void RenderScene::rebuildVisibleDrawLists ( const Core::Camera &camera ) const
  {
    m_visibleMainLines.clear ();
    m_visibleMainTexts.clear ();
    m_visibleOverlayLines.clear ();
    m_visibleOverlayTexts.clear ();

    const Math::BoxAABB viewport = camera.viewportBox ();

    for ( const auto &page : m_pages )
    {
      if ( ! page.bbox.intersects ( viewport ) ) continue;

      m_visibleMainLines.append ( page.lineFirst, page.lineCount );
      m_visibleMainTexts.append ( page.textFirst, page.textCount );
    }

    for ( const auto &placement : m_overlayPlacements )
    {
      if ( ! placement.bbox.intersects ( viewport ) ) continue;

      m_visibleOverlayLines.append ( placement.lineFirst, placement.lineCount );
      m_visibleOverlayTexts.append ( placement.textFirst, placement.textCount );
    }
  }

  RenderScene::GeometrySpan RenderScene::buildGeometry ( const Entity::Entity &entity,
                                                         Core::Font &font ) {
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
        const double cosAngle = std::cos ( text.rotation () );
        const double sinAngle = std::sin ( text.rotation () );

        auto transform = [&] ( const glm::dvec2 &local ) -> glm::vec2
        {
          return glm::vec2 ( text.position () +
                             glm::dvec2 ( local.x * cosAngle - local.y * sinAngle,
                                          local.x * sinAngle + local.y * cosAngle ) );
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

            const glm::vec2 p0 = transform ( { qMin.x, qMin.y } );
            const glm::vec2 p1 = transform ( { qMax.x, qMin.y } );
            const glm::vec2 p2 = transform ( { qMax.x, qMax.y } );
            const glm::vec2 p3 = transform ( { qMin.x, qMax.y } );

            geometry.textVertices.push_back (
                { p0, { glyph.uvMin.x, glyph.uvMin.y }, color, renderKey } );
            geometry.textVertices.push_back (
                { p1, { glyph.uvMax.x, glyph.uvMin.y }, color, renderKey } );
            geometry.textVertices.push_back (
                { p2, { glyph.uvMax.x, glyph.uvMax.y }, color, renderKey } );
            geometry.textVertices.push_back (
                { p0, { glyph.uvMin.x, glyph.uvMin.y }, color, renderKey } );
            geometry.textVertices.push_back (
                { p2, { glyph.uvMax.x, glyph.uvMax.y }, color, renderKey } );
            geometry.textVertices.push_back (
                { p3, { glyph.uvMin.x, glyph.uvMax.y }, color, renderKey } );
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
           m_overlayTextVertices.size () > kMaxOverlayTextVertices;
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
} // namespace Qadra::Render
