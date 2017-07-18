
#ifndef UTIL_GUI_HIDEAPPLYBOX_HH
#define UTIL_GUI_HIDEAPPLYBOX_HH 1

#include <QWidget>
#include <memory>

namespace Ui {
class HideApplyBox;
}

class HideApplyBox : public QWidget
{
  Q_OBJECT;

public:
  HideApplyBox(QWidget* parent);
  ~HideApplyBox();

  void setCanApply(bool enabled);

protected:
  virtual void changeEvent(QEvent *event);

Q_SIGNALS:
  void apply();
  void hide();

private Q_SLOTS:
  void hideApply();

private:
  std::unique_ptr<Ui::HideApplyBox> ui;
};

#endif // UTIL_GUI_HIDEAPPLYBOX_HH
