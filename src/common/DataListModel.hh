
#ifndef DataListModel_hh
#define DataListModel_hh 1

#include "ObsTableModel.hh"
#include <vector>

class DataListModel : public ObsTableModel
{ Q_OBJECT;
public:
  typedef std::vector<Sensor> Sensors_t;
  typedef std::vector<timeutil::ptime> Times_t;

  DataListModel(EditAccessPtr eda, const TimeRange& limits);
  ~DataListModel();

  virtual void insertColumn(int before, ObsColumnPtr c);
  virtual void removeColumn(int at);

  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  virtual QVariant columnHeader(int section, Qt::Orientation orientation, int role) const;
  virtual timeutil::ptime timeAtRow(int row) const;

  QModelIndexList findIndexes(const SensorTime& st);

  void setCenter(int stationId)
    { mCenter = stationId; }
  
protected:
  virtual int rowAtTime(const timeutil::ptime& time) const;
  virtual int rowOrColumnCount(bool timeDirection) const;

private:
  Times_t mTimes;
  int mCenter;
};

#endif // DataListModel_hh
