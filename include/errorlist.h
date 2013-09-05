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

#include "hqcdefs.h"
#include "KvalobsData.h"

#include <kvalobs/kvData.h>

#include <QtCore/QString>
#include <QtGui/QTableView>

#include <set>
#include <vector>

class ErrorListTableModel;
class HqcMainWindow;
class miMessage;
class QWidget;

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
    ErrorList(const std::vector<int>& selectedParameters,
              const timeutil::ptime&,
              const timeutil::ptime&,
              QWidget*,
              int,
              model::KvalobsDataListPtr,
              const std::vector<modDatl>&);
    virtual ~ErrorList();


    enum mem_change { NO_CHANGE, CORR_OK, ORIG_OK, INTERPOLATED, REDISTRIBUTED, CORRECTED, REJECTED };
    struct mem {
        double orig;
        double corr;
        double morig;
        kvalobs::kvControlInfo controlinfo;
        kvalobs::kvUseInfo useinfo;
        std::string cfailed;
        int flg;
        int sen;
        int lev;
        int flTyp;
        int parNo;
        int stnr;
        QString name;
        timeutil::ptime obstime;
        timeutil::ptime tbtime;
        int typeId;
        
        mem_change change;
        float changed_value;
        bool changed_qc2allowed;
        mem() : change(NO_CHANGE), changed_value(0), changed_qc2allowed(false) { }
    };

    kvalobs::kvData getKvData() const;

    bool maybeSave();
    
public Q_SLOTS:
    void saveChanges();

protected:
    void closeEvent ( QCloseEvent * event );
    
Q_SIGNALS:
    void errorListClosed();
    void signalNavigateTo(const kvalobs::kvData&);

private:
    void makeErrorList(const std::vector<int>& selectedParameters,
                       const timeutil::ptime& stime,
                       const timeutil::ptime& etime,
                       int lity,
                       model::KvalobsDataListPtr dtl,
                       const std::vector<modDatl>& mdtl);

    void fillMemoryStores(const std::vector<int>& selectedParameters,
                          const timeutil::ptime& stime,
                          const timeutil::ptime& etime,
                          int lity,
                          model::KvalobsDataListPtr dtl,
                          const std::vector<modDatl>& mdtl);

    /*!
     * \brief Constructs a kvData object from a memory store object
     */
    kvalobs::kvData getKvData(const mem &m) const;
    kvalobs::kvData getKvData(int row) const
        { return getKvData(getMem(row)); }
    const mem& getMem(int row) const;

    int getSelectedRow() const;
                                    
private Q_SLOTS:
    void showFail(const QModelIndex& index);

    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:
    void showSameStation();
    void signalStationSelected();

private:
    HqcMainWindow* mainWindow;
    int mLastSelectedRow;
    
    std::auto_ptr<ErrorListTableModel> mTableModel;
    
    std::vector<mem> memStore2;
};

#endif
