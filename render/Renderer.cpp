#include "Renderer.hpp"

#include <QFile>
#include <QTextStream>

namespace Qadra::Render
{
  void Renderer::init ( const QString &shaderDir )
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

  void Renderer::render ( const Cad::Document &document, const Core::Camera &camera ) const
  {
    const auto &order = document.drawOrder ();
    if ( order.isEmpty () ) return;

    std::vector<LinePass::Vertex> lineVertices;
    const float maxDepth = static_cast<float> ( order.size () );

    for ( int i = 0; i < order.size (); ++i )
    {
      auto *entity = document.find ( order[i] );
      if ( ! entity ) continue;

      const float depth = static_cast<float> ( i ) / maxDepth;

      constexpr glm::dvec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };

      switch ( entity->type () )
      {
        case Entity::EntityType::Line:
        {
          const auto *line = static_cast<Entity::Line *> ( entity );
          lineVertices.push_back ( { line->start (), color, depth } );
          lineVertices.push_back ( { line->end (), color, depth } );
          break;
        }
        default:
          break;
      }
    }

    m_linePass.render ( camera, lineVertices );
  }
} // namespace Qadra::Render
