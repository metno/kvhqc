/* -*- c++ -*-
   HQC - Free Software for Manual Quality Control of Meteorological Observations

   Copyright (C) 2014 met.no

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

#ifndef HQC_SEARCHWINDOW_H
#define HQC_SEARCHWINDOW_H

#define ENABLE_ERRORLIST 1
//#define ENABLE_EXTREMES 1
//#define ENABLE_MISSINGOBS 1
//#define ENABLE_REJECTEDOBS 1
//#define ENABLE_TEXTDATA 1
//#define ENABLE_DIANA 1
#define ENABLE_SIMPLECORRECTIONS 1

#include "common/AbstractReinserter.hh"
#include "common/ObsAccess.hh"
#include "util/timeutil.hh"

#include <QString>
#include <QMainWindow>

#include <memory>
#include <set>

class HelpDialog;
QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QSettings;
class QSignalMapper;
class QSplitter;
QT_END_NAMESPACE

class AutoDataList;
class NavigationHistory;
class SensorTime;
class SimpleCorrections;
class StationDataList;
class TimeSeriesView;

#ifdef ENABLE_ERRORLIST
class ErrorList;
#endif

namespace Ui {
class SearchWindow;
}

class SearchWindow : public QMainWindow
{ Q_OBJECT;

public:
  SearchWindow(QWidget* parent=0);
  ~SearchWindow();

  void writeSettings();
  void readSettings();

protected:
  virtual void changeEvent(QEvent *event);
 
private Q_SLOTS:
  void navigateTo(const SensorTime& st);

  void onActivateSearchTab(int index);
  void onActivateDataTab(int index);

private:
  void addTab(QWidget* widget, QString keys);
  void activateTab(QTabWidget* tabs, int index);
  void retranslateUi();
  void retranslateTabs(QTabWidget* tabs);
  void setupSearchTabs();
  void setupDataTabs();
  QSplitter* splitterDataSearch() const;

private:
  QTabWidget* mTabsData;
  QTabWidget* mTabsSearch;

  SensorTime mLastNavigated;

#ifdef ENABLE_DIANA
  ClientButton* pluginB;
  std::auto_ptr<HqcDianaHelper> mDianaHelper;
#endif

  QSplitter* mSplitterDataPlot;
  AutoDataList* mAutoDataList;
  TimeSeriesView* mTimeSeriesView;

#ifdef ENABLE_ERRORLIST
  ErrorList* mErrorsView;
#endif
#ifdef ENABLE_EXTREMES
  ExtremesView* mExtremesView;
#endif
#ifdef ENABLE_MISSINGOBS
  MissingView* mMissingView;
#endif
  NavigationHistory* mNavigationHistory;

  StationDataList* mStationData;
#ifdef ENABLE_SIMPLECORRECTIONS
  SimpleCorrections* mCorrections;
#endif

  QSignalMapper *mActivateSearchTab, *mActivateDataTab;
  int mId;

private: // static
  static int findId();
  static void releaseId(int id);

  typedef std::set<int> int_s;
  static int_s sUsedIds;
};

#endif // HQC_SEARCHWINDOW_H
