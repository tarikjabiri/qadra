#ifndef QADRA_UI_DRAW_MENU_HPP
#define QADRA_UI_DRAW_MENU_HPP

#include "ToolKind.hpp"

#include <QMenu>

class QAction;
class QActionGroup;

namespace Qadra::Ui
{
  class DrawMenu : public QMenu
  {
    Q_OBJECT

  public:
    explicit DrawMenu ( QWidget *parent = nullptr );

    ~DrawMenu () override;

    void setSelectedToolKind ( Qadra::Tool::ToolKind kind );

    [[nodiscard]] Qadra::Tool::ToolKind selectedToolKind () const noexcept;

  signals:
    void toolSelected ( Qadra::Tool::ToolKind kind );

  private:
    QActionGroup *m_actionGroup{};
    QAction *m_lineAction{};
    QAction *m_polylineAction{};
    QAction *m_arcAction{};
    QAction *m_circleAction{};
    QAction *m_ellipseAction{};
    QAction *m_textAction{};
  };
} // namespace Qadra::Ui

#endif // QADRA_UI_DRAW_MENU_HPP
