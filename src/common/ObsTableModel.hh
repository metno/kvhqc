
#ifndef OBSTABLEMODEL_HH
#define OBSTABLEMODEL_HH 1

#include "ObsColumn.hh"
#include "common/EditAccess.hh"
#include "TimeSpan.hh"

#include <QtCore/QAbstractTableModel>

class ObsTableModel : public QAbstractTableModel
{   Q_OBJECT;
public:
  ObsTableModel(EditAccess_p kda, const TimeSpan& time, int step = (24*60*60));
  virtual ~ObsTableModel();

  virtual int rowCount(const QModelIndex&) const;
  virtual int columnCount(const QModelIndex&) const;
  virtual Qt::ItemFlags flags(const QModelIndex& index) const;
  virtual QVariant data(const QModelIndex& index, int role) const;
  virtual bool setData(const QModelIndex& index, const QVariant& value, int role);
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  virtual QVariant columnHeader(int section, Qt::Orientation orientation, int role) const;

  virtual timeutil::ptime timeAtRow(int row) const;
  virtual SensorTime findSensorTime(const QModelIndex& idx) const;

  virtual void setTimeInRows(bool tir);

  virtual ObsColumn_p getColumn(int idx) const
    { return mColumns[idx]; }
  virtual void insertColumn(int before, ObsColumn_p c);
  virtual void removeColumn(int at);
  virtual void moveColumn(int from, int to);

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

protected:
  virtual int rowAtTime(const timeutil::ptime& time) const;
  virtual int rowOrColumnCount(bool timeDirection) const;
  virtual bool isTimeOrientation(Qt::Orientation orientation) const;

  virtual void beginInsertR(int first, int last);
  virtual void beginInsertC(int first, int last);
  virtual void endInsertR();
  virtual void endInsertC();

  virtual void beginRemoveR(int first, int last);
  virtual void beginRemoveC(int first, int last);
  virtual void endRemoveR();
  virtual void endRemoveC();

  virtual void updateTimes();

private Q_SLOTS:
  void onColumnChanged(const timeutil::ptime& time, ObsColumn_p column);
  void onColumnTimesChanged(ObsColumn_p column);

private:
  int timeIndex(const QModelIndex& index) const
    { return mTimeInRows ? index.row() : index.column(); }
  int columnIndex(const QModelIndex& index) const
    { return mTimeInRows ? index.column() : index.row(); }

protected:
  EditAccess_p mDA;
  bool mTimeInRows;
  TimeSpan mTime;
  int mTimeStep; //! time step between rows, in seconds
  timeutil::ptime mTime0; //! start time rounded to timeStep
  int mRowCount; //! number of rows after rounding

private:
  ObsColumn_pv mColumns;
};

#endif /* OBSTABLEMODEL_HH */
