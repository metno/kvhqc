
#ifndef TimespanDataList_hh
#define TimespanDataList_hh 1

#include "DataList.hh"
#include "common/ObsColumn.hh"

class QMenu;
class QToolButton;
class QSignalMapper;

// ------------------------------------------------------------------------

class TimespanDataList : public DataList
{ Q_OBJECT
public:
  TimespanDataList(QWidget* parent=0);
  ~TimespanDataList();
  
protected:
  SensorTime sensorSwitch() const;
  void switchSensorPrepare();
  void loadChangesXML(const QDomElement& doc_changes);
  void switchSensorDone();
  void storeChangesXML(QDomElement& doc_changes);

  virtual void updateModel() = 0;
  void retranslateUi();

  const TimeSpan& timeSpan() const
    { return mTimeLimits; }

private Q_SLOTS:
  void onChangeStart(QObject*);
  void onChangeEnd(QObject*);
  void onResetTime();

private:
  QSignalMapper* mMapperStart;
  QSignalMapper* mMapperEnd;
  QMenu* mMenuTime;
  QMenu* mMenuStart;
  QMenu* mMenuEnd;
  QToolButton* mButtonTime;
  QList<QAction*> mTimeActions;
  QAction* mActionResetTime;

  TimeSpan mTimeLimits, mOriginalTimeLimits;
};

#endif // TimespanDataList_hh
