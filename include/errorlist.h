/* -*- c++ -*-
HQC - Free Software for Manual Quality Control of Meteorological Observations

Copyright (C) 2013 met.no

Contact information:
Norwegian Meteorological Institute
Box 43 Blindern
0313 OSLO
NORWAY
email: kvalobs-dev@met.no

This file is part of HQC

HQC is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

HQC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with HQC; if not, write to the Free Software Foundation Inc.,
51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef ERRORLIST_H
#define ERRORLIST_H

#include "AnalyseErrors.hh"
#include "KvalobsData.h"
#include "ModelAccess.hh"
#include "TimeRange.hh"

#include <QtCore/QString>
#include <QtGui/QTableView>

#include <set>
#include <vector>

class ErrorListTableModel;
class HqcMainWindow;
QT_BEGIN_NAMESPACE;
class QSortFilterProxyModel;
class QWidget;
QT_END_NAMESPACE;

/**
 * \brief The error list. i.e. list of observations with error flags.
 *
 * \detailed The error list consists of two parts.  One part holds the data for the observations
 * which are flagged as erronous.  The other part has cells where the user can insert
 * new values or approve or reject existing values.
 */

class ErrorList : public QTableView
{ Q_OBJECT;
public:
    typedef std::vector<EditDataPtr> Errors_t;
    typedef std::vector<Sensor> Sensors_t;

    ErrorList(QWidget* parent=0);
    ~ErrorList();

    void setDataAccess(EditAccessPtr eda, ModelAccessPtr mda);
    void setSensorsAndTimes(const Sensors_t& sensors, const TimeRange& limits, bool errorsForSalen);

    EditDataPtr getObs() const;

Q_SIGNALS:
    void errorListClosed();
    void signalNavigateTo(const SensorTime& st);

private:
    void onDataChanged(ObsAccess::ObsDataChange, ObsDataPtr);
    EditDataPtr getObs(int row) const;
    int getSelectedRow() const;
                                    
private Q_SLOTS:
    void showFail(const QModelIndex& index);

    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:
    void resizeHeaders();
    void unsubscribeAll();
    void showSameStation();
    void signalStationSelected();

private:
    HqcMainWindow* mainWindow;
    int mLastSelectedRow;

    EditAccessPtr mDA;
    ModelAccessPtr mMA;

    typedef std::vector<ObsSubscription> Subscriptions_t;
    Subscriptions_t mSubscriptions;

    std::auto_ptr<QSortFilterProxyModel> mSortProxy;
    std::auto_ptr<ErrorListTableModel> mTableModel;
};

#endif
