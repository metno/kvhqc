// -*- c++ -*-

#ifndef ERRORLISTTABLE_H
#define ERRORLISTTABLE_H

#include "errorlist.h"

#include <QtCore/QAbstractTableModel>
#include <map>

class ErrorListTableModel : public QAbstractTableModel
{ Q_OBJECT;
public:
    ErrorListTableModel(const std::vector<ErrorList::mem>& errorList);
    ~ErrorListTableModel();

    enum EDIT_COLUMNS {
        COL_STATION_ID = 0,
        COL_STATION_NAME,
        COL_OBS_MONTH,
        COL_OBS_DAY,
        COL_OBS_HOUR,
        COL_OBS_MINUTE,
        COL_OBS_PARAM,
        COL_OBS_TYPEID,
        COL_OBS_ORIG,
        COL_OBS_CORR,
        COL_OBS_MODEL,
        COL_OBS_FLAG_NAME,
        COL_OBS_FLAG_EQ,
        COL_OBS_FLAG_VAL,
        COL_CORR_OK,
        COL_ORIG_OK,
        COL_INTERPOLATED,
        COL_CORRECTED,
        COL_REJECTED,
        NCOLUMNS
    };

    virtual int rowCount(const QModelIndex&) const;
    virtual int columnCount(const QModelIndex&) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual QVariant data(const QModelIndex& index, int role) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role);
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    void showSameStation(int stationID);
    const ErrorList::mem& mem4Row(int row) const
        { return mErrorList.at(row); }

    const std::vector<ErrorList::mem>& errorList() const
        { return mErrorList; }

private:
    std::vector<ErrorList::mem> mErrorList;
    int mShowStation;
};

#endif
