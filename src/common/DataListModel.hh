
#ifndef DataListModel_hh
#define DataListModel_hh 1

#include "ObsTableModel.hh"
#include <vector>

class DataListModel : public ObsTableModel
{ Q_OBJECT;
public:
  DataListModel(EditAccess_p eda, QObject* parent=0);
  ~DataListModel();

  QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  QVariant columnHeader(int section, Qt::Orientation orientation, int role) const;
  timeutil::ptime timeAtRow(int row) const;

  QModelIndexList findIndexes(const SensorTime& st);

  /*! Set station in the center.
   *  Headers for other stations should show distance to this station.
   */
  virtual void setCenter(int stationId);

  virtual void setTimeStep(int step);

  /*! Enables or disables time filtering by getTimeStep().
   */
  virtual void setFilterByTimestep(bool fbts);

Q_SIGNALS:
  void changedCenter(int center);
  void changedFilterByTimestep(bool enabled, bool ftbs);
  
protected:
  int rowAtTime(const timeutil::ptime& time) const;
  int countTimes() const;
  void updateTimes();

private:
  //! all times from all columns, possibly filtered by getTimeStep()
  Time_v mTimes;

  //! \see setFilterByTimestep
  bool mFilterByTimestep;

  //! \see setCenter()
  int mCenter;
};

#endif // DataListModel_hh
