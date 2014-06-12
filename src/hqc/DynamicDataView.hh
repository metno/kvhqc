
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
  /*! Uses sensorSwitch() to determine if the store sensor has
   *  changed. If yes, updates storeSensorTime() and calls doSensorSwitch().
   */
  void doNavigateTo();

  /*! Store changes, calling storeChangesXML() to build XML description.
   */
  void storeChanges();

  /*! Load changes, calling loadChangesXML() to extract changes from
   *  XML iff there are stored changes.
   */
  void loadChanges();

  /*! Starts actual sensor switch.
   */
  virtual void doSensorSwitch();

  virtual std::string viewType() const = 0;
  virtual std::string viewId() const;

  /*! Derives store sensor from current sensor. May be simpler,
   *  e.g. not depending on parameter id. Default is to return current
   *  sensor.
   */
  virtual SensorTime sensorSwitch() const;
  
  /*! Parse changes XML document to replay changes for store sensor.
   */
  virtual void loadChangesXML(const QDomElement& doc_changes);

  /*! Store changes for store sensor to XML document. Empty document
   *  mean no changes. Default does nothing.
   */
  virtual void storeChangesXML(QDomElement& doc_changes);

  const SensorTime& storeSensorTime() const
    { return mStoreST; }

private:
  SensorTime mStoreST;
};

#endif // DynamicDataView_hh
