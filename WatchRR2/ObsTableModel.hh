
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

    timeutil::ptime timeAtRow(int row) const;

protected:
    void addColumn(ObsColumnPtr c);
    ObsColumnPtr getColumn(int idx) const
        { return mColumns[idx]; }

    int rowAtTime(const timeutil::ptime& time) const;

private:
    void onColumnChanged(const timeutil::ptime& time, ObsColumn* column);

protected:
    EditAccessPtr mDA;
    TimeRange mTime;

private:
    std::vector<ObsColumnPtr> mColumns;
};

#endif /* OBSTABLEMODEL_HH */
