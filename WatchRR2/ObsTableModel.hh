
#ifndef OBSTABLEMODEL_HH
#define OBSTABLEMODEL_HH 1

#include "ObsColumn.hh"
#include "EditAccess.hh"
#include "TimeRange.hh"

#include <QtCore/QAbstractTableModel>

class ObsTableModel : public QAbstractTableModel
{   Q_OBJECT;
public:
    ObsTableModel(EditAccessPtr kda, const TimeRange& time);
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

    virtual void insertColumn(int before, ObsColumnPtr c);
    void addColumn(ObsColumnPtr c)
        { insertColumn(mColumns.size(), c); }
    virtual ObsColumnPtr getColumn(int idx) const
        { return mColumns[idx]; }

protected:
    virtual int rowAtTime(const timeutil::ptime& time) const;
    virtual int rowOrColumnCount(bool timeDirection) const;

private:
    void onColumnChanged(const timeutil::ptime& time, ObsColumn* column);
    int timeIndex(const QModelIndex& index) const
        { return mTimeInRows ? index.row() : index.column(); }
    int columnIndex(const QModelIndex& index) const
        { return mTimeInRows ? index.column() : index.row(); }

protected:
    EditAccessPtr mDA;
    TimeRange mTime;
    bool mTimeInRows;

private:
    std::vector<ObsColumnPtr> mColumns;
};

#endif /* OBSTABLEMODEL_HH */
