#ifndef QADRA_CAD_HISTORY_DOCUMENT_EDITOR_HPP
#define QADRA_CAD_HISTORY_DOCUMENT_EDITOR_HPP

#include "Document.hpp"
#include "EntitySnapshot.hpp"

#include <QString>
#include <QUndoCommand>
#include <QUndoStack>
#include <memory>
#include <utility>
#include <vector>

namespace Qadra::Cad
{
  class DocumentEditor
  {
  public:
    class HistoryCommand : public QUndoCommand
    {
    public:
      explicit HistoryCommand ( QString text ) { setText ( std::move ( text ) ); }

      void applyNow () { apply (); }

      void skipNextRedo () noexcept { m_skipNextRedo = true; }

      void redo () override
      {
        if ( m_skipNextRedo )
        {
          m_skipNextRedo = false;
          return;
        }

        apply ();
      }

      void undo () override { revert (); }

    protected:
      virtual void apply () = 0;
      virtual void revert () = 0;

    private:
      bool m_skipNextRedo = false;
    };

    explicit DocumentEditor ( Document &document );
    ~DocumentEditor ();

    DocumentEditor ( const DocumentEditor & ) = delete;
    DocumentEditor &operator= ( const DocumentEditor & ) = delete;
    DocumentEditor ( DocumentEditor && ) = delete;
    DocumentEditor &operator= ( DocumentEditor && ) = delete;

    [[nodiscard]] Document &document () noexcept { return m_document; }

    [[nodiscard]] const Document &document () const noexcept { return m_document; }

    void setFont ( Core::Font *font ) noexcept { m_font = font; }

    [[nodiscard]] bool hasFont () const noexcept { return m_font != nullptr; }

    [[nodiscard]] QUndoStack &undoStack () noexcept { return m_undoStack; }

    [[nodiscard]] const QUndoStack &undoStack () const noexcept { return m_undoStack; }

    Core::Handle addLine ( const Entity::LineRecord &record );
    Core::Handle addArc ( const Entity::ArcRecord &record );
    Core::Handle addCircle ( const Entity::CircleRecord &record );
    Core::Handle addEllipse ( const Entity::EllipseRecord &record );
    Core::Handle addLWPolyline ( const Entity::LWPolylineRecord &record );
    Core::Handle addText ( const Entity::TextRecord &record );

    bool removeEntity ( Core::Handle handle );

    Core::Handle loadLine ( const Entity::LineRecord &record );
    Core::Handle loadArc ( const Entity::ArcRecord &record );
    Core::Handle loadCircle ( const Entity::CircleRecord &record );
    Core::Handle loadEllipse ( const Entity::EllipseRecord &record );
    Core::Handle loadLWPolyline ( const Entity::LWPolylineRecord &record );
    Core::Handle loadText ( const Entity::TextRecord &record );

    void beginMacro ( const QString &text );
    void endMacro ();
    void clearHistory ();

  private:
    class InsertEntityCommand;
    class RemoveEntityCommand;
    class CompositeHistoryCommand;

    struct PendingMacro
    {
      QString text;
      std::vector<std::unique_ptr<HistoryCommand>> commands;
      int depth = 1;
    };

    [[nodiscard]] std::unique_ptr<HistoryCommand> makeAddCommand ( EntitySnapshot snapshot );
    [[nodiscard]] std::unique_ptr<HistoryCommand> makeRemoveCommand ( EntitySnapshot snapshot );
    [[nodiscard]] bool restoreSnapshot ( const EntitySnapshot &snapshot );
    [[nodiscard]] std::optional<EntitySnapshot> takeSnapshot ( Core::Handle handle );
    [[nodiscard]] std::optional<EntitySnapshot> snapshotEntity ( Core::Handle handle ) const;

    void push ( std::unique_ptr<HistoryCommand> command );

    Document &m_document;
    Core::Font *m_font = nullptr;
    QUndoStack m_undoStack;
    std::unique_ptr<PendingMacro> m_pendingMacro;
  };
} // namespace Qadra::Cad

#endif // QADRA_CAD_HISTORY_DOCUMENT_EDITOR_HPP
