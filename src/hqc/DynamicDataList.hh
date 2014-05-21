
#ifndef DynamicDataList_hh
#define DynamicDataList_hh 1

#include "DataList.hh"
#include "common/ObsColumn.hh"

class QPushButton;
class QDomElement;

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
  void resetTimeSpan()
    { mTimeLimits = mOriginalTimeLimits; }

private Q_SLOTS:
  void onEarlier();
  void onLater();

private:
  std::string changesXML();
  void replayXML(const std::string& changes);

private:
  QPushButton* mButtonEarlier;
  QPushButton* mButtonLater;

  TimeSpan mTimeLimits, mOriginalTimeLimits;

  SensorTime mStoreSensorTime;
};

#endif // DynamicDataList_hh
