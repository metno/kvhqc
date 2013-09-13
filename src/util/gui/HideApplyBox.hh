
#ifndef UTIL_GUI_HIDEAPPLYBOX_HH
#define UTIL_GUI_HIDEAPPLYBOX_HH 1

#include <QtGui/QWidget>
#include <memory>

namespace Ui {
class HideApplyBox;
}

class HideApplyBox : public QWidget
{
  Q_OBJECT;

public:
  HideApplyBox(QWidget* parent);

  void setCanApply(bool enabled);

Q_SIGNALS:
  void apply();
  void hide();

private Q_SLOTS:
  void hideApply();

private:
  std::auto_ptr<Ui::HideApplyBox> ui;
};

#endif // UTIL_GUI_HIDEAPPLYBOX_HH
