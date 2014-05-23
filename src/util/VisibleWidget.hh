
#ifndef VisibleWidget_hh
#define VisibleWidget_hh 1

#include <QWidget>

class VisibleWidget : public QWidget
{ Q_OBJECT
public:
  VisibleWidget(QWidget* parent=0);
  ~VisibleWidget() = 0;
                 
Q_SIGNALS:
  void visibilityUpdate(bool visible);
  
protected:
  virtual void showEvent(QShowEvent* showEvent);
  virtual void hideEvent(QHideEvent* hideEvent);
  virtual void resizeEvent(QResizeEvent *resizeEvent);
  virtual void changeEvent(QEvent *event);

  virtual void retranslateUi();

private:
  void sendUpdate();

private:
  bool mShown, mEmpty;
};

#endif // VisibleWidget_hh
