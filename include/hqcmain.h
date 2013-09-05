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
#ifndef HQCMAIN_H
#define HQCMAIN_H

#include "HqcDataReinserter.h"
#include "ObsAccess.hh"
#include "timeutil.hh"

#include <QtCore/QString>
#include <QtGui/QMainWindow>

#include <memory>

class ClientButton;
class HelpDialog;
QT_BEGIN_NAMESPACE
class QAction;
class QMdiArea;
class QMdiSubWindow;
class QTimer;
QT_END_NAMESPACE

class AutoColumnView;
class DataList;
class DianaShowDialog;
class EditVersionModel;
class ExtremesView;
class HintWidget;
class HqcDianaHelper;
class KvalobsAccess;
class KvalobsModelAccess;
class EditAccess;
class ListDialog;
class RejectDialog;
class SensorTime;
class TimeSeriesView;
class TextDataDialog;

namespace Ui {
class HqcMainWindow;
}

class HqcMainWindow : public QMainWindow
{ Q_OBJECT;

public:
    HqcMainWindow();
    ~HqcMainWindow();

    void startup(const QString& captionSuffix);

    HqcReinserter* getReinserter()
        { return reinserter; }

    void setReinserter(HqcReinserter* r, const QString& userName);

protected:
    void moveEvent(QMoveEvent* event);
    void resizeEvent(QResizeEvent* event);
    void closeEvent(QCloseEvent* event);

Q_SIGNALS:
    void newStationList(std::vector<QString>&);
    void newParameterList(const std::vector<int>&);
    void printErrorList();

private Q_SLOTS:
    void errListMenu();
    void allListMenu();
    void errLogMenu();
    void dataListMenu();
    void errLisaMenu();
    void allListSalenMenu();
    void ListOK();
    void rejectedOK();
    void textDataOK();
    void dianaShowOK();

    void startKro();
    void screenshot();
    void helpUse();
    void helpNews();
    void helpFlag();
    void helpParam();
    void about();
    void aboutQt();

    //! bring up the WatchRR dialog
    void showWatchRR();

    //! bring up the WatchWeather dialog
    void showWeather();

    void onVersionCheckTimeout();
    void onSaveChanges();
    void onUndoChanges();
    void onRedoChanges();

  void onShowExtremes();
  void onShowErrorList();
  void onShowChanges();
  void onShowSimpleCorrections();

  void onTabCloseRequested(int index);

private:
    void navigateTo(const SensorTime& st);
    void onDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr obs);

    //! Retrieves from stlist information about a given station
    void findStationInfo(int, QString&, double&, double&, double&, int&, int&);

    enum listType {erLi, erLo, daLi, erSa, alLi, alSa, dumLi};
    void listMenu(listType lt);

    void exitNoKvalobs();
    void writeSettings();
    void readSettings();
    void checkVersionSettings();

    void readFromStation();

private:
    ListDialog* lstdlg;
    DianaShowDialog* dshdlg;
    TextDataDialog* txtdlg;
    RejectDialog* rejdlg;
    HelpDialog* mHelpDialog;

    listType lity;

    HqcReinserter* reinserter;

    /// True after first time ListOk() have been invoked with valid input
    bool listExist;

    QString userName;

    std::auto_ptr<Ui::HqcMainWindow> ui;

    QTimer* mVersionCheckTimer;
    HintWidget* mHints;

    boost::shared_ptr<KvalobsAccess> kda;
    boost::shared_ptr<KvalobsModelAccess> kma;
    boost::shared_ptr<EditAccess> eda;

    std::auto_ptr<EditVersionModel> mEditVersions;

    TimeSeriesView* mTimeSeriesView;

    ClientButton* pluginB;
    std::auto_ptr<HqcDianaHelper> mDianaHelper;

    std::auto_ptr<AutoColumnView> mAutoColumnView;
    DataList* mAutoDataList;

  ExtremesView* mExtremesView;
};

#endif
