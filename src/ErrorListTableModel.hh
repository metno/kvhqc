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
