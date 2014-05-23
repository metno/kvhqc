
#ifndef DynamicDataView_hh
#define DynamicDataView_hh 1

#include "AbstractDataView.hh"

class QDomElement;

class DynamicDataView : public AbstractDataView
{ Q_OBJECT
public:
  DynamicDataView(QWidget* parent=0);
  ~DynamicDataView();
  
protected:
  void doNavigateTo();

  void storeChanges();

  virtual std::string viewType() const;
  virtual std::string viewId() const;

  virtual SensorTime sensorSwitch() const;
  virtual void switchSensorPrepare();
  virtual void loadChangesXML(const QDomElement& doc_changes);
  virtual void switchSensorDone();
  virtual void storeChangesXML(QDomElement& doc_changes);

  const SensorTime& storeSensorTime() const
    { return mStoreST; }

private:
  void loadChanges();

private:
  SensorTime mStoreST;
};

#endif // DynamicDataView_hh
