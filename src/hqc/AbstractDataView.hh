
#ifndef AbstractDataView_hh
#define AbstractDataView_hh 1

#include "common/EditAccess.hh"
#include "common/ModelAccess.hh"
#include "common/NavigateHelper.hh"
#include "common/Sensor.hh"
#include <QtGui/QTableView>

class AbstractDataView : public QWidget
{ Q_OBJECT
public:
  AbstractDataView(QWidget* parent=0);
  ~AbstractDataView() = 0;
                 
  void setDataAccess(EditAccess_p eda, ModelAccess_p ma);

public Q_SLOTS:  
  void navigateTo(const SensorTime&);
  
Q_SIGNALS:
  void signalNavigateTo(const SensorTime&);
  
protected:
  virtual void showEvent(QShowEvent* showEvent);
  virtual void hideEvent(QHideEvent* hideEvent);
  virtual void resizeEvent(QResizeEvent *resizeEvent);
  virtual void changeEvent(QEvent *event);

  const SensorTime& currentSensorTime() const
    { return mNavigate.current(); }
  virtual void doNavigateTo() = 0;
  virtual void retranslateUi();
  virtual void sendNavigateTo(const SensorTime& st);

protected:
  NavigateVisible mNavigate;
};

#endif // AbstractDataView_hh
