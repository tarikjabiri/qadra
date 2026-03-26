#ifndef QADRA_UI_DRAW_TOOL_BAR_HPP
#define QADRA_UI_DRAW_TOOL_BAR_HPP

#include <QToolBar>

class QAction;

namespace Qadra::Ui
{
  class DrawToolBar : public QToolBar
  {
    Q_OBJECT

  public:
    explicit DrawToolBar ( QWidget *parent = nullptr );

    ~DrawToolBar () override;

  private:
    QAction *m_lineAction{};
    QAction *m_arcAction{};
    QAction *m_textAction{};
  };
} // namespace Qadra::Ui

#endif // QADRA_UI_DRAW_TOOL_BAR_HPP
