#include "DocumentEditor.hpp"

#include <utility>

namespace Qadra::Cad
{
  namespace
  {
    [[nodiscard]] QString entityLabel ( const Entity::EntityType type )
    {
      switch ( type )
      {
        case Entity::EntityType::Line:
          return QStringLiteral ( "Line" );
        case Entity::EntityType::Arc:
          return QStringLiteral ( "Arc" );
        case Entity::EntityType::LWPolyline:
          return QStringLiteral ( "Polyline" );
        case Entity::EntityType::Circle:
          return QStringLiteral ( "Circle" );
        case Entity::EntityType::Ellipse:
          return QStringLiteral ( "Ellipse" );
        case Entity::EntityType::Text:
          return QStringLiteral ( "Text" );
      }

      return QStringLiteral ( "Entity" );
    }
  } // namespace

  class DocumentEditor::InsertEntityCommand final : public DocumentEditor::HistoryCommand
  {
  public:
    InsertEntityCommand ( DocumentEditor &editor, EntitySnapshot snapshot )
        : HistoryCommand ( QStringLiteral ( "Add %1" ).arg ( entityLabel ( snapshot.type () ) ) ),
          m_editor ( editor ), m_snapshot ( std::move ( snapshot ) )
    {
    }

  private:
    void apply () override
    {
      [[maybe_unused]] const bool restored = m_editor.restoreSnapshot ( m_snapshot );
    }

    void revert () override
    {
      [[maybe_unused]] const auto removed = m_editor.takeSnapshot ( m_snapshot.handle );
    }

    DocumentEditor &m_editor;
    EntitySnapshot m_snapshot;
  };

  class DocumentEditor::RemoveEntityCommand final : public DocumentEditor::HistoryCommand
  {
  public:
    RemoveEntityCommand ( DocumentEditor &editor, EntitySnapshot snapshot )
        : HistoryCommand (
              QStringLiteral ( "Delete %1" ).arg ( entityLabel ( snapshot.type () ) ) ),
          m_editor ( editor ), m_snapshot ( std::move ( snapshot ) )
    {
    }

  private:
    void apply () override
    {
      [[maybe_unused]] const auto removed = m_editor.takeSnapshot ( m_snapshot.handle );
    }

    void revert () override
    {
      [[maybe_unused]] const bool restored = m_editor.restoreSnapshot ( m_snapshot );
    }

    DocumentEditor &m_editor;
    EntitySnapshot m_snapshot;
  };

  class DocumentEditor::CompositeHistoryCommand final : public DocumentEditor::HistoryCommand
  {
  public:
    CompositeHistoryCommand (
        QString text, std::vector<std::unique_ptr<DocumentEditor::HistoryCommand>> commands )
        : HistoryCommand ( std::move ( text ) ), m_commands ( std::move ( commands ) )
    {
    }

  private:
    void apply () override
    {
      for ( auto &command : m_commands ) command->redo ();
    }

    void revert () override
    {
      for ( auto it = m_commands.rbegin (); it != m_commands.rend (); ++it ) ( *it )->undo ();
    }

    std::vector<std::unique_ptr<DocumentEditor::HistoryCommand>> m_commands;
  };

  DocumentEditor::DocumentEditor ( Document &document ) : m_document ( document ) { }

  DocumentEditor::~DocumentEditor () = default;

  Core::Handle DocumentEditor::addLine ( const Entity::LineRecord &record )
  {
    EntitySnapshot snapshot{
        .handle = m_document.next (),
        .renderKey = m_document.m_renderKeySeed + 1,
        .drawIndex = static_cast<std::size_t> ( m_document.drawOrder ().size () ),
        .payload = record,
    };
    const Core::Handle handle = snapshot.handle;

    push ( makeAddCommand ( std::move ( snapshot ) ) );
    return handle;
  }

  Core::Handle DocumentEditor::addArc ( const Entity::ArcRecord &record )
  {
    EntitySnapshot snapshot{
        .handle = m_document.next (),
        .renderKey = m_document.m_renderKeySeed + 1,
        .drawIndex = static_cast<std::size_t> ( m_document.drawOrder ().size () ),
        .payload = record,
    };
    const Core::Handle handle = snapshot.handle;

    push ( makeAddCommand ( std::move ( snapshot ) ) );
    return handle;
  }

  Core::Handle DocumentEditor::addCircle ( const Entity::CircleRecord &record )
  {
    EntitySnapshot snapshot{
        .handle = m_document.next (),
        .renderKey = m_document.m_renderKeySeed + 1,
        .drawIndex = static_cast<std::size_t> ( m_document.drawOrder ().size () ),
        .payload = record,
    };
    const Core::Handle handle = snapshot.handle;

    push ( makeAddCommand ( std::move ( snapshot ) ) );
    return handle;
  }

  Core::Handle DocumentEditor::addEllipse ( const Entity::EllipseRecord &record )
  {
    EntitySnapshot snapshot{
        .handle = m_document.next (),
        .renderKey = m_document.m_renderKeySeed + 1,
        .drawIndex = static_cast<std::size_t> ( m_document.drawOrder ().size () ),
        .payload = record,
    };
    const Core::Handle handle = snapshot.handle;

    push ( makeAddCommand ( std::move ( snapshot ) ) );
    return handle;
  }

  Core::Handle DocumentEditor::addLWPolyline ( const Entity::LWPolylineRecord &record )
  {
    EntitySnapshot snapshot{
        .handle = m_document.next (),
        .renderKey = m_document.m_renderKeySeed + 1,
        .drawIndex = static_cast<std::size_t> ( m_document.drawOrder ().size () ),
        .payload = record,
    };
    const Core::Handle handle = snapshot.handle;

    push ( makeAddCommand ( std::move ( snapshot ) ) );
    return handle;
  }

  Core::Handle DocumentEditor::addText ( const Entity::TextRecord &record )
  {
    if ( ! m_font ) return {};

    const auto layout = m_font->layout ( record.text );
    const auto bbox = Document::computeTextBBox ( record, layout, *m_font );
    EntitySnapshot snapshot{
        .handle = m_document.next (),
        .renderKey = m_document.m_renderKeySeed + 1,
        .drawIndex = static_cast<std::size_t> ( m_document.drawOrder ().size () ),
        .payload =
            TextEntitySnapshot{
                .record = record,
                .layout = layout,
                .bbox = bbox,
            },
    };
    const Core::Handle handle = snapshot.handle;

    push ( makeAddCommand ( std::move ( snapshot ) ) );
    return handle;
  }

  bool DocumentEditor::removeEntity ( const Core::Handle handle )
  {
    auto snapshot = snapshotEntity ( handle );
    if ( ! snapshot.has_value () ) return false;

    push ( makeRemoveCommand ( std::move ( *snapshot ) ) );
    return true;
  }

  Core::Handle DocumentEditor::loadLine ( const Entity::LineRecord &record )
  {
    return m_document.addLine ( record );
  }

  Core::Handle DocumentEditor::loadArc ( const Entity::ArcRecord &record )
  {
    return m_document.addArc ( record );
  }

  Core::Handle DocumentEditor::loadCircle ( const Entity::CircleRecord &record )
  {
    return m_document.addCircle ( record );
  }

  Core::Handle DocumentEditor::loadEllipse ( const Entity::EllipseRecord &record )
  {
    return m_document.addEllipse ( record );
  }

  Core::Handle DocumentEditor::loadLWPolyline ( const Entity::LWPolylineRecord &record )
  {
    return m_document.addLWPolyline ( record );
  }

  Core::Handle DocumentEditor::loadText ( const Entity::TextRecord &record )
  {
    if ( ! m_font ) return {};
    return m_document.addText ( record, *m_font );
  }

  void DocumentEditor::beginMacro ( const QString &text )
  {
    if ( m_pendingMacro )
    {
      ++m_pendingMacro->depth;
      return;
    }

    m_pendingMacro = std::make_unique<PendingMacro> ();
    m_pendingMacro->text = text;
  }

  void DocumentEditor::endMacro ()
  {
    if ( ! m_pendingMacro ) return;
    if ( --m_pendingMacro->depth > 0 ) return;

    auto pendingMacro = std::move ( m_pendingMacro );
    if ( pendingMacro->commands.empty () ) return;

    if ( pendingMacro->commands.size () == 1 )
    {
      auto command = std::move ( pendingMacro->commands.front () );
      command->setText ( pendingMacro->text );
      command->skipNextRedo ();
      m_undoStack.push ( command.release () );
      return;
    }

    auto command = std::make_unique<CompositeHistoryCommand> (
        pendingMacro->text, std::move ( pendingMacro->commands ) );
    command->skipNextRedo ();
    m_undoStack.push ( command.release () );
  }

  void DocumentEditor::clearHistory ()
  {
    m_pendingMacro.reset ();
    m_undoStack.clear ();
  }

  std::unique_ptr<DocumentEditor::HistoryCommand>
  DocumentEditor::makeAddCommand ( EntitySnapshot snapshot )
  {
    return std::make_unique<InsertEntityCommand> ( *this, std::move ( snapshot ) );
  }

  std::unique_ptr<DocumentEditor::HistoryCommand>
  DocumentEditor::makeRemoveCommand ( EntitySnapshot snapshot )
  {
    return std::make_unique<RemoveEntityCommand> ( *this, std::move ( snapshot ) );
  }

  bool DocumentEditor::restoreSnapshot ( const EntitySnapshot &snapshot )
  {
    return m_document.restoreEntity ( snapshot );
  }

  std::optional<EntitySnapshot> DocumentEditor::takeSnapshot ( const Core::Handle handle )
  {
    return m_document.takeEntity ( handle );
  }

  std::optional<EntitySnapshot> DocumentEditor::snapshotEntity ( const Core::Handle handle ) const
  {
    return m_document.snapshotEntity ( handle );
  }

  void DocumentEditor::push ( std::unique_ptr<HistoryCommand> command )
  {
    if ( ! command ) return;

    if ( m_pendingMacro )
    {
      command->applyNow ();
      m_pendingMacro->commands.push_back ( std::move ( command ) );
      return;
    }

    m_undoStack.push ( command.release () );
  }
} // namespace Qadra::Cad
