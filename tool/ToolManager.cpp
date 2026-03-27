#include "ToolManager.hpp"

#include "Tool.hpp"
#include "cad/history/DocumentEditor.hpp"

#include <utility>

namespace
{
  [[nodiscard]] QString macroTextForKind ( const Qadra::Tool::ToolKind kind )
  {
    switch ( kind )
    {
      case Qadra::Tool::ToolKind::Line:
        return QStringLiteral ( "Line" );
      case Qadra::Tool::ToolKind::Polyline:
        return QStringLiteral ( "Polyline" );
      case Qadra::Tool::ToolKind::Arc:
        return QStringLiteral ( "Arc" );
      case Qadra::Tool::ToolKind::Circle:
        return QStringLiteral ( "Circle" );
      case Qadra::Tool::ToolKind::Ellipse:
        return QStringLiteral ( "Ellipse" );
      case Qadra::Tool::ToolKind::Text:
        return QStringLiteral ( "Text" );
      case Qadra::Tool::ToolKind::None:
        break;
    }

    return QStringLiteral ( "Tool" );
  }
} // namespace

namespace Qadra::Tool
{
  ToolManager::~ToolManager () = default;

  ToolEventResult ToolManager::setActiveTool ( std::unique_ptr<Tool> tool,
                                               const ToolContext &context )
  {
    if ( ! m_activeTool && ! tool ) return ToolEventResult::ignored ();

    if ( m_activeTool )
    {
      m_activeTool->deactivate ( context );
      context.editor.endMacro ();
    }

    m_activeTool = std::move ( tool );

    if ( m_activeTool )
    {
      context.editor.beginMacro ( macroTextForKind ( m_activeTool->kind () ) );
      m_activeTool->activate ( context );
    }

    return ToolEventResult::handledAndRepaint ();
  }

  ToolEventResult ToolManager::clearActiveTool ( const ToolContext &context )
  {
    if ( ! m_activeTool ) return ToolEventResult::ignored ();

    m_activeTool->deactivate ( context );
    m_activeTool.reset ();
    context.editor.endMacro ();

    return ToolEventResult::handledAndRepaint ();
  }

  ToolEventResult ToolManager::handlePointerPress ( const ToolContext &context,
                                                    const ToolPointerEvent &event )
  {
    if ( ! m_activeTool ) return ToolEventResult::ignored ();
    return m_activeTool->onPointerPress ( context, event );
  }

  ToolEventResult ToolManager::handlePointerMove ( const ToolContext &context,
                                                   const ToolPointerEvent &event )
  {
    if ( ! m_activeTool ) return ToolEventResult::ignored ();
    return m_activeTool->onPointerMove ( context, event );
  }

  ToolEventResult ToolManager::handlePointerRelease ( const ToolContext &context,
                                                      const ToolPointerEvent &event )
  {
    if ( ! m_activeTool ) return ToolEventResult::ignored ();
    return m_activeTool->onPointerRelease ( context, event );
  }

  CommandResult ToolManager::handleCommand ( const ToolContext &context,
                                             const ToolCommand &command )
  {
    if ( ! m_activeTool )
      return CommandResult::rejected ( "No active tool to receive command input." );
    return m_activeTool->onCommand ( context, command );
  }

  ToolEventResult ToolManager::cancelActiveTool ( const ToolContext &context )
  {
    if ( ! m_activeTool ) return ToolEventResult::ignored ();

    const ToolEventResult result = m_activeTool->cancel ( context );
    context.editor.endMacro ();
    context.editor.beginMacro ( macroTextForKind ( m_activeTool->kind () ) );
    return result;
  }

  ToolKind ToolManager::activeToolKind () const noexcept
  {
    return m_activeTool ? m_activeTool->kind () : ToolKind::None;
  }

  ToolPreview ToolManager::preview ( const ToolContext &context ) const
  {
    if ( ! m_activeTool ) return {};
    return m_activeTool->preview ( context );
  }

  std::string ToolManager::prompt () const
  {
    return m_activeTool ? m_activeTool->prompt () : std::string{};
  }

  const Tool *ToolManager::activeTool () const noexcept { return m_activeTool.get (); }
} // namespace Qadra::Tool
