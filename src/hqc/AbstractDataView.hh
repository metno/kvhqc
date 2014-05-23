
#ifndef AbstractDataView_hh
#define AbstractDataView_hh 1

#include "common/NavigateHelper.hh"
#include "util/VisibleWidget.hh"

class AbstractDataView : public VisibleWidget
{ Q_OBJECT
public:
  AbstractDataView(QWidget* parent=0);
  ~AbstractDataView();
                 
public Q_SLOTS:  
  virtual void navigateTo(const SensorTime&);
  
Q_SIGNALS:
  void signalNavigateTo(const SensorTime&);

private Q_SLOTS:
  virtual void onVisibilityUpdate(bool visible);
  
protected:
  const SensorTime& currentSensorTime() const
    { return mNavigate.current(); }
  
  //! Subclass should navigate to new sensor or time or both
  virtual void doNavigateTo() = 0;

  void sendNavigateTo(const SensorTime& st);

protected:
  NavigateVisible mNavigate;
};

#endif // AbstractDataView_hh
