
#ifndef QNoCloseMdiSubWindow_hh
#define QNoCloseMdiSubWindow_hh 1

#include <QtGui/QCloseEvent>
#include <QtGui/QMdiSubWindow>

class QNoCloseMdiSubWindow : public QMdiSubWindow {
public:
  QNoCloseMdiSubWindow(QWidget* parent=0)
    : QMdiSubWindow(parent) { }
  virtual void closeEvent(QCloseEvent* ce)
    { ce->ignore(); }
};

#endif // QNoCloseMdiSubWindow_hh
