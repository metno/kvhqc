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

#ifndef HQC_HQCMAINWINDOW_H
#define HQC_HQCMAINWINDOW_H

#define ENABLE_ERRORLIST 1
//#define ENABLE_EXTREMES 1
//#define ENABLE_MISSINGOBS 1
//#define ENABLE_REJECTEDOBS 1
//#define ENABLE_TEXTDATA 1
//#define ENABLE_DIANA 1

#include "common/HqcDataReinserter.hh"
#include "common/ObsAccess.hh"
#include "util/timeutil.hh"

#include <QtCore/QString>
#include <QtGui/QMainWindow>

#include <memory>

class HelpDialog;
QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QMdiArea;
class QMdiSubWindow;
class QSplitter;
class QTimer;
QT_END_NAMESPACE

class AutoDataList;
class CachingAccess;
class EditAccess;
class EditVersionModel;
class EtaProgressDialog;
class HintWidget;
class JumpToObservation;
class KvalobsAccess;
class KvalobsModelAccess;
class ListDialog;
class SensorTime;
class TimeSeriesView;

#ifdef ENABLE_ERRORLIST
class ErrorList;
#endif

namespace Ui {
class HqcMainWindow;
}

class HqcMainWindow : public QMainWindow
{ Q_OBJECT;

public:
  HqcMainWindow();
  ~HqcMainWindow();

  void startup(const QString& captionSuffix);

  void setReinserter(HqcReinserter* r);

protected:
  virtual void moveEvent(QMoveEvent* event);
  virtual void resizeEvent(QResizeEvent* event);
  virtual void closeEvent(QCloseEvent* event);
  virtual void changeEvent(QEvent *event);
 
Q_SIGNALS:
  void newStationList(std::vector<QString>&);
  void newParameterList(const std::vector<int>&);
  void printErrorList();

private Q_SLOTS:
  void errListMenu();
  void allListMenu();
  void dataListMenu();
  void errLisaMenu();
  void allListSalenMenu();
  void ListOK();
  void rejectedOK();
  void textDataOK();

  void startKro();
  void screenshot();
  void helpUse();
  void helpNews();
  void helpFlag();
  void helpParam();
  void about();
  void aboutQt();
  void onUserSettings();

  void kvalobsAvailable(bool);

  void showWatchRR();
  void showWeather();

  void onVersionCheckTimeout();
  void onSaveChanges();
  void onUndoChanges();
  void onRedoChanges();

  void onShowChanges();
  void onShowSimpleCorrections();

  void onJumpToObservation();

  void onTabCloseRequested(int index);

  void navigateTo(const SensorTime& st);

  void onEditVersionChanged(size_t current, size_t highest);

private:
  enum listType {erLi, daLi, erSa, alLi, alSa, dumLi};
  void listMenu(listType lt);

  void writeSettings();
  void readSettings();
  void checkVersionSettings();

  void onKvalobsFetchingData(int total, int ready);

private:
  ListDialog* lstdlg;
#ifdef ENABLE_TEXTDATA
  TextDataDialog* txtdlg;
#endif
#ifdef ENABLE_REJECTEDOBS
  RejectedObsDialog* rejdlg;
#endif
  HelpDialog* mHelpDialog;
  JumpToObservation* mJumpToObservation;

  listType lity;

  /// True after first time ListOk() have been invoked with valid input
  bool listExist;

  SensorTime mLastNavigated;

  std::auto_ptr<Ui::HqcMainWindow> ui;
  QSplitter* mAutoViewSplitter;

  QTimer* mVersionCheckTimer;
  HintWidget* mHints;

  boost::shared_ptr<KvalobsAccess> kda;
  boost::shared_ptr<CachingAccess> cda;
  boost::shared_ptr<KvalobsModelAccess> kma;
  boost::shared_ptr<EditAccess> eda;

  std::auto_ptr<EditVersionModel> mEditVersions;

#ifdef ENABLE_DIANA
  ClientButton* pluginB;
  std::auto_ptr<HqcDianaHelper> mDianaHelper;
#endif

  QLabel* mKvalobsAvailable;

  EtaProgressDialog* mProgressDialog;

  AutoDataList* mAutoDataList;
  TimeSeriesView* mTimeSeriesView;

#ifdef ENABLE_ERRORLIST
  ErrorList* mErrorsView;
  QDockWidget* mDockErrors;
#endif
#ifdef ENABLE_EXTREMES
  ExtremesView* mExtremesView;
  QDockWidget* mDockExtremes;
#endif
#ifdef ENABLE_MISSINGOBS
  MissingView* mMissingView;
  QDockWidget* mDockMissing;
#endif
};

#endif // HQC_HQCMAINWINDOW_H
