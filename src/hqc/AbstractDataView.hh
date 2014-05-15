
#ifndef AbstractDataView_hh
#define AbstractDataView_hh 1

#include "common/EditAccess.hh"
#include "common/ModelAccess.hh"
#include "common/NavigateHelper.hh"
#include "common/Sensor.hh"
#include "util/VisibleWidget.hh"

class AbstractDataView : public VisibleWidget
{ Q_OBJECT
public:
  AbstractDataView(QWidget* parent=0);
  ~AbstractDataView() = 0;
                 
  void setDataAccess(EditAccess_p eda, ModelAccess_p ma);

public Q_SLOTS:  
  void navigateTo(const SensorTime&);
  
Q_SIGNALS:
  void signalNavigateTo(const SensorTime&);

private Q_SLOTS:
  virtual void onVisibilityUpdate(bool visible);
  
protected:
  const SensorTime& currentSensorTime() const
    { return mNavigate.current(); }
  virtual void doNavigateTo() = 0;
  virtual void sendNavigateTo(const SensorTime& st);

protected:
  NavigateVisible mNavigate;
};

#endif // AbstractDataView_hh
