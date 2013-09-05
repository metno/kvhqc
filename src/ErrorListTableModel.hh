// -*- c++ -*-

#ifndef ERRORLISTTABLE_H
#define ERRORLISTTABLE_H

#include "AnalyseErrors.hh"
#include "EditAccess.hh"
#include "ModelAccess.hh"

#include <QtCore/QAbstractTableModel>

#include <vector>

class ErrorListTableModel : public QAbstractTableModel
{   Q_OBJECT;
public:
    ErrorListTableModel(EditAccessPtr eda, ModelAccessPtr mda, const Errors::Errors_t& errorList, bool errorsForSalen);
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
        COL_OBS_FLAGS,
        NCOLUMNS
    };

    virtual int rowCount(const QModelIndex&) const;
    virtual int columnCount(const QModelIndex&) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual QVariant data(const QModelIndex& index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    void showSameStation(int stationID);
    EditDataPtr mem4Row(int row) const
        { return (row>=0 and row<(int)mErrorList.size()) ? mErrorList.at(row).obs : EditDataPtr(); }

    const Errors::Errors_t& errorList() const
        { return mErrorList; }

private:
    void onDataChanged(ObsAccess::ObsDataChange, ObsDataPtr);

private:
    EditAccessPtr mDA;
    ModelAccessPtr mMA;
    Errors::Errors_t mErrorList;
    bool mErrorsForSalen;
    int mShowStation;
};

#endif
