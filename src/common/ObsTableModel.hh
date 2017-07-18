
#ifndef OBSTABLEMODEL_HH
#define OBSTABLEMODEL_HH 1

#include "ObsColumn.hh"
#include "common/EditAccess.hh"
#include "TimeSpan.hh"

#include <QAbstractTableModel>

class ObsTableModel : public QAbstractTableModel
{   Q_OBJECT;
public:
  ObsTableModel(EditAccess_p da, int step = (24*60*60), QObject* parent=0);
  virtual ~ObsTableModel();

  virtual int rowCount(const QModelIndex&) const;
  virtual int columnCount(const QModelIndex&) const;
  virtual Qt::ItemFlags flags(const QModelIndex& index) const;
  virtual QVariant data(const QModelIndex& index, int role) const;
  virtual bool setData(const QModelIndex& index, const QVariant& value, int role);
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  virtual QVariant columnHeader(int section, Qt::Orientation orientation, int role) const;

  virtual void setTimeSpan(const TimeSpan& limits);
  virtual timeutil::ptime timeAtRow(int row) const;
  virtual SensorTime findSensorTime(const QModelIndex& idx) const;

  virtual void setTimeInRows(bool tir);

  virtual int countTimes() const;
  virtual int countColumns() const;

  virtual ObsColumn_p getColumn(int idx) const;
  virtual void insertColumn(int before, ObsColumn_p c);
  virtual void removeColumn(int at);
  virtual void moveColumn(int from, int to);
  virtual void removeAllColumns();

  void addColumn(ObsColumn_p c)
    { insertColumn(mColumns.size(), c); }

  /*! Set time difference between rows.
   * \param step time step in seconds
   */
  virtual void setTimeStep(int step);

  /*! Get time difference between rows.
   * \return time step in seconds
   */
  int getTimeStep() const
    { return mTimeStep; }

Q_SIGNALS:
  void changedTimeStep(int step);
  void changedTimeInRows(bool tir);
  void busyStatus(bool busy);

protected:
  virtual int rowAtTime(const timeutil::ptime& time) const;
  virtual bool isTimeOrientation(Qt::Orientation orientation) const;

  virtual void timeInsertBegin(int first, int last);
  virtual void timeInsertEnd();
  virtual void columnInsertBegin(int first, int last);
  virtual void columnInsertEnd();

  virtual void timeRemoveBegin(int first, int last);
  virtual void timeRemoveEnd();
  virtual void columnRemoveBegin(int first, int last);
  virtual void columnRemoveEnd();

  virtual void updateTimes();

  //! check if there are som busy columns; emit busyStatus if send==true or if changed
  void countBusyColumns(bool send);

private Q_SLOTS:
  virtual void onColumnChanged(const timeutil::ptime& time, ObsColumn_p column);
  virtual void onColumnTimesChanged(ObsColumn_p column);
  virtual void onColumnBusyStatus(bool);

private:
  int timeIndex(const QModelIndex& index) const
    { return mTimeInRows ? index.row() : index.column(); }
  int columnIndex(const QModelIndex& index) const
    { return mTimeInRows ? index.column() : index.row(); }
  void detachColumn(ObsColumn_p column);

protected:
  EditAccess_p mDA;
  bool mTimeInRows;
  TimeSpan mTime;
  int mTimeStep; //! time step between rows, in seconds
  timeutil::ptime mTime0; //! start time rounded to timeStep

private:
  ObsColumn_pv mColumns;
  int mTimeCount; //! number of times after rounding
  bool mHaveBusyColumns;
};

#endif /* OBSTABLEMODEL_HH */
