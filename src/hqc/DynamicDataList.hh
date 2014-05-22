
#ifndef DynamicDataList_hh
#define DynamicDataList_hh 1

#include "DataList.hh"
#include "common/ObsColumn.hh"

class QDomElement;
class QMenu;
class QToolButton;
class QSignalMapper;

// ------------------------------------------------------------------------

class DynamicDataList : public DataList
{ Q_OBJECT
public:
  DynamicDataList(QWidget* parent=0);
  ~DynamicDataList();
  
protected:
  void doNavigateTo();

  void storeChanges();
  virtual std::string viewType() const = 0;
  virtual std::string viewId() const;

  virtual DataListModel* makeModel() = 0;
  virtual void changes(QDomElement& doc_changes);
  virtual void replay(const QDomElement& doc_changes);
  void retranslateUi();

  const TimeSpan& timeSpan() const
    { return mTimeLimits; }

private Q_SLOTS:
  void onChangeStart(QObject*);
  void onChangeEnd(QObject*);
  void onResetTime();

private:
  std::string changesXML();
  void replayXML(const std::string& changes);

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

  SensorTime mStoreSensorTime;
};

#endif // DynamicDataList_hh
