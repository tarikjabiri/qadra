#ifndef QADRA_UI_DRAW_TOOL_BAR_HPP
#define QADRA_UI_DRAW_TOOL_BAR_HPP

#include "ToolKind.hpp"

#include <QToolBar>

class QAction;
class QActionGroup;

namespace Qadra::Ui
{
  class DrawToolBar : public QToolBar
  {
    Q_OBJECT

  public:
    explicit DrawToolBar ( QWidget *parent = nullptr );

    ~DrawToolBar () override;

    void setSelectedToolKind ( Qadra::Tool::ToolKind kind );

    [[nodiscard]] Qadra::Tool::ToolKind selectedToolKind () const noexcept;

  signals:
    void toolSelected ( Qadra::Tool::ToolKind kind );

  private:
    QActionGroup *m_actionGroup{};
    QAction *m_lineAction{};
    QAction *m_arcAction{};
    QAction *m_textAction{};
  };
} // namespace Qadra::Ui

#endif // QADRA_UI_DRAW_TOOL_BAR_HPP
