#ifndef QADRA_UI_QUICK_TOOL_BAR_HPP
#define QADRA_UI_QUICK_TOOL_BAR_HPP

#include <QToolBar>

class QAction;

namespace Qadra::Ui
{
  class QuickToolBar : public QToolBar
  {
    Q_OBJECT

  public:
    explicit QuickToolBar ( QWidget *parent = nullptr );

    ~QuickToolBar () override;

    void addQuickAction ( QAction *action );

    void addQuickSeparator ();
  };
} // namespace Qadra::Ui

#endif // QADRA_UI_QUICK_TOOL_BAR_HPP
