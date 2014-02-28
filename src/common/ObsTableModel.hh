
#ifndef OBSTABLEMODEL_HH
#define OBSTABLEMODEL_HH 1

#include "ObsColumn.hh"
#include "EditAccess.hh"
#include "TimeRange.hh"

#include <QtCore/QAbstractTableModel>

class ObsTableModel : public QAbstractTableModel
{   Q_OBJECT;
public:
  ObsTableModel(EditAccessPtr kda, const TimeRange& time, int step = (24*60*60));
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

  void setTimeInRows(bool tir)
    { mTimeInRows = tir; }

  virtual ObsColumnPtr getColumn(int idx) const
    { return mColumns[idx]; }
  virtual void insertColumn(int before, ObsColumnPtr c);
  virtual void removeColumn(int at);
  virtual void moveColumn(int from, int to);

  void addColumn(ObsColumnPtr c)
    { insertColumn(mColumns.size(), c); }

  /*! Set time difference between rows.
   * \param step time step in seconds
   */
  void setTimeStep(int step);

  /*! Get time difference between rows.
   * \return time step in seconds
   */
  int getTimeStep() const
    { return mTimeStep; }

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

private:
  typedef std::vector<ObsColumnPtr> ObsColumns_t;

private:
  void onColumnChanged(const timeutil::ptime& time, ObsColumnPtr column);
  int timeIndex(const QModelIndex& index) const
    { return mTimeInRows ? index.row() : index.column(); }
  int columnIndex(const QModelIndex& index) const
    { return mTimeInRows ? index.column() : index.row(); }

protected:
  EditAccessPtr mDA;
  bool mTimeInRows;
  TimeRange mTime;
  int mTimeStep; //! time step between rows, in seconds
  timeutil::ptime mTime0; //! start time rounded to timeStep
  int mRowCount; //! number of rows after rounding

private:
  ObsColumns_t mColumns;
};

#endif /* OBSTABLEMODEL_HH */
