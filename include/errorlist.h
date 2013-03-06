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
class QMouseEvent;
class QPainter;
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
        std::string controlinfo;
        std::string useinfo;
        std::string cfailed;
        int flg;
        int sen;
        int lev;
        QString flTyp;
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

    struct missObs {
        timeutil::ptime oTime;
        int parno;
        int statno;
        int missNo;
    };
    
    kvalobs::kvData getKvData() const;

    std::vector<missObs> mList;
    std::vector<mem> missList;

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
    void makeMissingList(const std::vector<int>& selectedParameters,
                         const timeutil::ptime& stime,
                         const timeutil::ptime& etime,
                         model::KvalobsDataListPtr dtl);
    void fillMemoryStores(const std::vector<int>& selectedParameters,
                          const timeutil::ptime& stime,
                          const timeutil::ptime& etime,
                          int lity,
                          model::KvalobsDataListPtr dtl,
                          const std::vector<modDatl>& mdtl);

    /**
     * \brief Decide if an observation is going to the error list or not
     * \return The largest flag value from the automatic control, negative
     *         if no HQC control is indicated
     */
    int errorFilter(int, std::string, std::string, QString&);

    /**
     * \brief Decide if given parameter is to be controlled in HQC
     * \return TRUE if the parameter is to be controlled.
     */
    bool priorityParameterFilter(int);

    /**
     * \brief Decide if the given control is to be checked in HQC
     * \return 0 if only one control flag is set, and this is not
     *         to be checked in HQC
     */
    int priorityControlFilter(QString);

    /**
     * \brief Checks if given parameter can be stored at given time.
     */
    bool specialTimeFilter(int, const timeutil::ptime&);
    bool typeFilter(int, int, int, const timeutil::ptime&);

    /**
     * \brief Find which observations shall be moved from memory store 1 to error list
     */
    bool isErrorInMemstore1(const mem& m);

    /**
     * \brief Checks if a parameter has model values.
     *
     * \return TRUE if given parameter has model values
     */
    bool paramHasModel(int);

    /**
     * \brief Checks if a parameter is code.
     *
     * \return 0 if given parameter is code, 1 otherwise.
     */
    bool paramIsCode(int);

    /**
     * \brief Checks if an observation is in the missing list
     *
     * \return TRUE if observation is missing.
     */
    bool obsInMissList(const mem&);
    
    /*!
     * \brief Constructs a kvData object from a memory store object
     */
    kvalobs::kvData getKvData(const mem &m) const;
    kvalobs::kvData getKvData(int row) const
        { return getKvData(getMem(row)); }
    const mem& getMem(int row) const;

    int getSelectedRow() const;
                                    
private Q_SLOTS:
    void updateKvBase(const mem&);
    void showFail(const QModelIndex& index);

    void showSameStation();
    void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

    void setupMissingList();
    void execMissingList();

    void signalStationSelected();

private:
    HqcMainWindow* mainWindow;
    int mLastSelectedRow;
    
    std::auto_ptr<ErrorListTableModel> mTableModel;
    
    std::vector<mem> memStore1;
    std::vector<mem> memStore2;
};

#endif
