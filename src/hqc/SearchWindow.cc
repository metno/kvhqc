/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2014-2018 met.no

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

#include "SearchWindow.hh"

#include "config.h"

#include "AutoDataList.hh"
#include "NavigationHistory.hh"
#include "TimeSeriesView.hh"

#include "common/CachingAccess.hh"
#include "common/KvalobsAccess.hh"

#include "common/KvalobsModelAccess.hh"
#include "common/KvHelpers.hh"
#include "common/KvMetaDataBuffer.hh"
#include "common/KvServiceHelper.hh"
#include "common/HqcApplication.hh"

#include "util/Helpers.hh"
#include "util/hqc_paths.hh"
#include "util/timeutil.hh"

#ifdef ENABLE_ERRORLIST
#include "errorlist/ErrorList.hh"
#endif // ENABLE_ERRORLIST

#ifdef ENABLE_DIANA
#include "HqcDianaHelper.hh"
#include <coserver/ClientButton.h>
#endif // ENABLE_DIANA

#ifdef ENABLE_EXTREMES
#include "extremes/ExtremesView.hh"
#endif // ENABLE_EXTREMES

#ifdef ENABLE_MISSINGOBS
#include "missingobs/MissingView.hh"
#endif // ENABLE_MISSINGOBS

#include "StationDataList.hh"

#ifdef ENABLE_SIMPLECORRECTIONS
#include "SimpleCorrections.hh"
#endif // ENABLE_SIMPLECORRECTIONS

#include <QSettings>
#include <QShortcut>
#include <QSignalMapper>
#include <QSplitter>

#define MILOGGER_CATEGORY "kvhqc.SearchWindow"
#include "common/ObsLogging.hh"

namespace /*anonymous*/ {
const char SETTINGS_SEARCH_GROUP[] = "searchwindow_%1";
const char SETTING_SEARCH_GEOMETRY[] = "geometry";
const char SETTING_SEARCH_AUTOVIEW_SPLITTER[] = "autoview_slider";
const char SETTING_SEARCH_DATASEARCH_SPLITTER[] = "datasearch_slider";
const char SETTING_TAB_DATA[] = "tab_data";
const char SETTING_TAB_RELATED[] = "related";
const char SETTING_TAB_STATION[] = "station";
const char SETTING_TAB_SINGLE[] = "single";
const char SETTING_TAB_SEARCH[] = "tab_search";
const char SETTING_TAB_ERRORS[] = "errorlist";
const char SETTING_TAB_EXTREMES[] = "extremes";
const char SETTING_TAB_MISSING[] = "missingobs";
const char SETTING_TAB_RECENT[] = "recent";

QString settingsSearchGroup(int id)
{
  return QString(SETTINGS_SEARCH_GROUP).arg(id);
}
} // anonymous namespace

SearchWindow::SearchWindow(QWidget* parent)
  : QMainWindow(parent)
  , mActivateSearchTab(new QSignalMapper(this))
  , mActivateDataTab(new QSignalMapper(this))
  , mId(findId())
{
  METLIBS_LOG_TIME();
  setAttribute(Qt::WA_DeleteOnClose, true);
  setWindowTitle(tr("HQC Search %1").arg(hqcApp->kvalobsDBName()));
  setWindowIcon(QIcon("icons:hqc_logo.svg"));
  resize(975, 700);

  QSplitter* sDataSearch = new QSplitter(Qt::Vertical, this);
  setCentralWidget(sDataSearch);

  mTabsData = new QTabWidget(sDataSearch);
  mTabsSearch = new QTabWidget(sDataSearch);

  sDataSearch->addWidget(mTabsSearch);
  sDataSearch->addWidget(mTabsData);

  setupSearchTabs();
  setupDataTabs();

  connect(mActivateSearchTab, SIGNAL(mapped(int)),
      this, SLOT(onActivateSearchTab(int)));
  connect(mActivateDataTab, SIGNAL(mapped(int)),
      this, SLOT(onActivateDataTab(int)));

#ifdef ENABLE_DIANA
  pluginB = new ClientButton("hqc", "/usr/bin/coserver4", statusBar());
  statusBar()->addPermanentWidget(pluginB, 0);
  mDianaHelper.reset(new HqcDianaHelper(pluginB));
  mDianaHelper->setDataAccess(eda, kma);
  mDianaHelper->signalNavigateTo.connect(boost::bind(&SearchWindow::navigateTo, this, _1));
#endif

  readSettings();
}

SearchWindow::~SearchWindow()
{
  METLIBS_LOG_SCOPE();
  writeSettings();
  releaseId(mId);
}

void SearchWindow::setupSearchTabs()
{
  METLIBS_LOG_TIME();
#ifdef ENABLE_ERRORLIST
  mErrorsView = new ErrorList(mTabsSearch);
  addTab(mErrorsView, tr("Ctrl+F", "Error search tab shortcut"));
  connect(mErrorsView, SIGNAL(signalNavigateTo(const SensorTime&)),
      this, SLOT(navigateTo(const SensorTime&)));
  connect(this, SIGNAL(signalNavigateTo(const SensorTime&)),
      mErrorsView, SLOT(navigateTo(const SensorTime&)));
#endif // ENABLE_ERRORLIST

#ifdef ENABLE_EXTREMES
  mExtremesView = new ExtremesView(mTabsSearch);
  addTab(mExtremesView, tr("Ctrl+E", "Extremes tab shortcut"));
  connect(mExtremesView, SIGNAL(signalNavigateTo(const SensorTime&)),
      this, SLOT(navigateTo(const SensorTime&)));
#endif // ENABLE_EXTREMES

#ifdef ENABLE_MISSINGOBS
  mMissingView = new MissingView(mTabsSearch);
  addTab(mMissingView, tr("Ctrl+M", "Missing Data tab shortcut"));
  connect(mMissingView, SIGNAL(signalNavigateTo(const SensorTime&)),
      this, SLOT(navigateTo(const SensorTime&)));
#endif // ENABLE_MISSINGOBS

  mNavigationHistory = new NavigationHistory(mTabsSearch);
  addTab(mNavigationHistory, tr("Ctrl+R", "Recent tab shortcut"));
  connect(mNavigationHistory, SIGNAL(signalNavigateTo(const SensorTime&)),
      this, SLOT(navigateTo(const SensorTime&)));
  connect(this, SIGNAL(signalNavigateTo(const SensorTime&)),
      mNavigationHistory, SLOT(navigateTo(const SensorTime&)));
}

void SearchWindow::setupDataTabs()
{
  METLIBS_LOG_TIME();
  mSplitterDataPlot = new QSplitter(Qt::Horizontal, mTabsData);
  mSplitterDataPlot->setOpaqueResize(false);
  mSplitterDataPlot->setWindowTitle(tr("List/Series"));
  mSplitterDataPlot->setWindowIcon(QIcon("icons:timeseries.svg"));
  addTab(mSplitterDataPlot, tr("Ctrl+1", "List/Series tab shortcut"));
  
  mAutoDataList = new AutoDataList(mSplitterDataPlot);
  connect(mAutoDataList, SIGNAL(signalNavigateTo(const SensorTime&)),
      this, SLOT(navigateTo(const SensorTime&)));
  connect(this, SIGNAL(signalNavigateTo(const SensorTime&)),
      mAutoDataList, SLOT(navigateTo(const SensorTime&)));

  mTimeSeriesView = new TimeSeriesView(mSplitterDataPlot);
  connect(this, SIGNAL(signalNavigateTo(const SensorTime&)),
      mTimeSeriesView, SLOT(navigateTo(const SensorTime&)));

  mSplitterDataPlot->addWidget(mAutoDataList);
  mSplitterDataPlot->addWidget(mTimeSeriesView);

  mStationData = new StationDataList(mTabsData);
  connect(mStationData, SIGNAL(signalNavigateTo(const SensorTime&)),
      this, SLOT(navigateTo(const SensorTime&)));
  connect(this, SIGNAL(signalNavigateTo(const SensorTime&)),
      mStationData, SLOT(navigateTo(const SensorTime&)));
  addTab(mStationData, tr("Ctrl+2", "Station data tab shortcut"));

#ifdef ENABLE_SIMPLECORRECTIONS
  mCorrections = new SimpleCorrections(hqcApp->editAccess(), hqcApp->modelAccess(), mTabsData);
  connect(this, SIGNAL(signalNavigateTo(const SensorTime&)),
      mCorrections, SLOT(navigateTo(const SensorTime&)));
  addTab(mCorrections, tr("Ctrl+3", "Single Observation tab shortcut"));
#endif
}

void SearchWindow::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::LanguageChange)
    retranslateUi();
  QMainWindow::changeEvent(event);
}

void SearchWindow::addTab(QWidget* widget, QString keys)
{
  QTabWidget* tabs = static_cast<QTabWidget*>(widget->parent());
  tabs->addTab(widget, widget->windowIcon(), widget->windowTitle());

  QSignalMapper* sm = tabs == mTabsSearch ? mActivateSearchTab : mActivateDataTab;
  QShortcut* sc = new QShortcut(QKeySequence(keys), this);
  connect(sc, SIGNAL(activated()), sm, SLOT(map()));
  sm->setMapping(sc, tabs->count() - 1);
}

void SearchWindow::retranslateUi()
{
  retranslateTabs(mTabsData);
  retranslateTabs(mTabsSearch);
}

void SearchWindow::retranslateTabs(QTabWidget* tabs)
{
  for (int i=0; i<tabs->count(); ++i)
    tabs->setTabText(i, tabs->widget(i)->windowTitle());
}

void SearchWindow::navigateTo(const SensorTime& st)
{
  METLIBS_LOG_TIME();
  METLIBS_LOG_DEBUG(LOGVAL(st));

  if (mNavigate.go(st))
    Q_EMIT signalNavigateTo(st);
}

void SearchWindow::activateTab(QTabWidget* tabs, int index)
{
  METLIBS_LOG_SCOPE();
  tabs->setCurrentIndex(index);
  tabs->currentWidget()->setFocus(Qt::ShortcutFocusReason);
}

void SearchWindow::onActivateSearchTab(int index)
{
  activateTab(mTabsSearch, index);
}

void SearchWindow::onActivateDataTab(int index)
{
  activateTab(mTabsData, index);
}

void SearchWindow::writeSettings()
{
  QWidget* ws = mTabsSearch->currentWidget();
  QString tabSearch = SETTING_TAB_ERRORS;
  if (ws == mNavigationHistory)
    tabSearch = SETTING_TAB_RECENT;
#ifdef ENABLE_EXTREMES
  if (ws == mExtremesView)
    tabSearch = SETTING_TAB_EXTREMES;
#endif // ENABLE_EXTREMES
#ifdef ENABLE_MISSINGOBS
  if (ws == mMissingView)
    tabSearch = SETTING_TAB_MISSING;
#endif // ENABLE_MISSINGOBS

  QWidget* wd = mTabsData->currentWidget();
  QString tabData = SETTING_TAB_RELATED;
  if (wd == mStationData)
    tabData = SETTING_TAB_STATION;
  else if (wd == mCorrections)
    tabData = SETTING_TAB_SINGLE;

  QSettings settings;
  settings.beginGroup(settingsSearchGroup(mId));
  settings.setValue(SETTING_SEARCH_GEOMETRY, saveGeometry());
  settings.setValue(SETTING_SEARCH_AUTOVIEW_SPLITTER, mSplitterDataPlot->saveState());
  settings.setValue(SETTING_SEARCH_DATASEARCH_SPLITTER, splitterDataSearch()->saveState());
  settings.setValue(SETTING_TAB_SEARCH, tabSearch);
  settings.setValue(SETTING_TAB_DATA,   tabData);
  settings.endGroup();

  mErrorsView->saveSettings(settings);
  mNavigationHistory->saveSettings(settings);
}

void SearchWindow::readSettings()
{
  METLIBS_LOG_SCOPE();

  QSettings settings;
  settings.beginGroup(settingsSearchGroup(mId));
  if (not restoreGeometry(settings.value(SETTING_SEARCH_GEOMETRY).toByteArray()))
    METLIBS_LOG_INFO("cannot restore search window geometry");
  if (not mSplitterDataPlot->restoreState(settings.value(SETTING_SEARCH_AUTOVIEW_SPLITTER).toByteArray()))
    METLIBS_LOG_INFO("cannot restore autoview splitter positions");
  if (not splitterDataSearch()->restoreState(settings.value(SETTING_SEARCH_DATASEARCH_SPLITTER).toByteArray()))
    METLIBS_LOG_INFO("cannot restore data-search splitter positions");
  const QString tabSearch = settings.value(SETTING_TAB_SEARCH, SETTING_TAB_ERRORS).toString();
  const QString tabData   = settings.value(SETTING_TAB_DATA,   SETTING_TAB_RELATED).toString();
  settings.endGroup();

  mErrorsView->restoreSettings(settings);
  mNavigationHistory->restoreSettings(settings);

  if (tabSearch == SETTING_TAB_RECENT)
    mTabsSearch->setCurrentWidget(mNavigationHistory);
#ifdef ENABLE_EXTREMES
  else if (tabSearch == SETTING_TAB_EXTREMES)
    mTabsSearch->setCurrentWidget(mExtremesView);
#endif // ENABLE_EXTREMES
#ifdef ENABLE_MISSINGOBS
  else if (tabSearch == SETTING_TAB_MISSING)
    mTabsSearch->setCurrentWidget(mMissingView);
#endif // ENABLE_MISSINGOBS
  else
    mTabsSearch->setCurrentWidget(mErrorsView);

  if (tabData == SETTING_TAB_SINGLE)
    mTabsData->setCurrentWidget(mCorrections);
  else if (tabData == SETTING_TAB_STATION)
    mTabsData->setCurrentWidget(mStationData);
  else
    mTabsData->setCurrentWidget(mSplitterDataPlot);

  METLIBS_LOG_DEBUG(LOGVAL(tabSearch) << LOGVAL(mTabsSearch->currentIndex()));
  METLIBS_LOG_DEBUG(LOGVAL(tabData)   << LOGVAL(mTabsData->currentIndex()));
}

QSplitter* SearchWindow::splitterDataSearch() const
{
  return static_cast<QSplitter*>(centralWidget());
}

// static
int SearchWindow::findId()
{
  for (int id=0; id<1000; ++id) {
    if (sUsedIds.count(id) == 0) {
      sUsedIds.insert(id);
      return id;
    }
  }
  return 0;
}

// static
void SearchWindow::releaseId(int id)
{
  sUsedIds.erase(id);
}

// static
std::set<int> SearchWindow::sUsedIds;
