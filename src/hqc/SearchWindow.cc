/*
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
#include "util/BusyIndicator.hh"
#include "util/UiHelpers.hh"

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
#include "missing/MissingView.hh"
#endif // ENABLE_MISSINGOBS

#ifdef ENABLE_SIMPLECORRECTIONS
#include "SimpleCorrections.hh"
#endif // ENABLE_SIMPLECORRECTIONS

#include <QtCore/QSettings>
#include <QtCore/QSignalMapper>
#include <QtGui/QShortcut>
#include <QtGui/QSplitter>

#define MILOGGER_CATEGORY "kvhqc.SearchWindow"
#include "common/ObsLogging.hh"

namespace {

const char SETTINGS_SEARCH_GROUP[] = "searchwindow";
const char SETTING_SEARCH_GEOMETRY[] = "geometry";
const char SETTING_SEARCH_AUTOVIEW_SPLITTER[] = "autoview_slider";
const char SETTING_SEARCH_DATASEARCH_SPLITTER[] = "datasearch_slider";

} // anonymous namespace

SearchWindow::SearchWindow(const QString& kvalobsInstanceName, QWidget* parent)
  : QMainWindow(parent)
  , mActivateSearchTab(new QSignalMapper(this))
  , mActivateDataTab(new QSignalMapper(this))
{
  METLIBS_LOG_SCOPE();
  setWindowTitle(tr("HQC Search %1").arg(kvalobsInstanceName));
  setWindowIcon(QIcon("icons:hqc_logo.svg"));
  resize(975, 700);

  QSplitter* sDataSearch = new QSplitter(Qt::Vertical, this);
  setCentralWidget(sDataSearch);

  mTabsData = new QTabWidget(sDataSearch);
  mTabsSearch = new QTabWidget(sDataSearch);

  sDataSearch->addWidget(mTabsData);
  sDataSearch->addWidget(mTabsSearch);

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
}

void SearchWindow::setupSearchTabs()
{
#ifdef ENABLE_ERRORLIST
  mErrorsView = new ErrorList(mTabsSearch);
  addTab(mErrorsView, tr("Ctrl+F", "Error search tab shortcut"));
  connect(mErrorsView, SIGNAL(signalNavigateTo(const SensorTime&)),
      this, SLOT(navigateTo(const SensorTime&)));
#endif // ENABLE_ERRORLIST

#ifdef ENABLE_EXTREMES
  mExtremesView = new ExtremesView(this);
  mExtremesView->signalNavigateTo.connect(boost::bind(&SearchWindow::navigateTo, this, _1));
  addTab(mExtremesView);
#endif // ENABLE_EXTREMES

#ifdef ENABLE_MISSINGOBS
  mMissingView = new MissingView(mTabsSearch);
  mMissingView->signalNavigateTo.connect(boost::bind(&SearchWindow::navigateTo, this, _1));
  addTab(mTabsSearch, mMissingView);
#endif // ENABLE_MISSINGOBS

  mNavigationHistory = new NavigationHistory(mTabsSearch);
  addTab(mNavigationHistory, tr("Ctrl+R", "Recent tab shortcut"));
  connect(mNavigationHistory, SIGNAL(signalNavigateTo(const SensorTime&)),
      this, SLOT(navigateTo(const SensorTime&)));
}

void SearchWindow::setupDataTabs()
{
  mSplitterDataPlot = new QSplitter(Qt::Horizontal, mTabsData);
  mSplitterDataPlot->setOpaqueResize(false);
  mSplitterDataPlot->setWindowTitle(tr("Auto List/Series"));
  mSplitterDataPlot->setWindowIcon(QIcon("icons:timeseries.svg"));
  addTab(mSplitterDataPlot, tr("Ctrl+1", "Auto List tab shortcut"));
  
  mAutoDataList = new AutoDataList(mSplitterDataPlot);
  connect(mAutoDataList, SIGNAL(signalNavigateTo(const SensorTime&)),
      this, SLOT(navigateTo(const SensorTime&)));

  mTimeSeriesView = new TimeSeriesView(mSplitterDataPlot);

  mSplitterDataPlot->addWidget(mAutoDataList);
  mSplitterDataPlot->addWidget(mTimeSeriesView);

#ifdef ENABLE_SIMPLECORRECTIONS
  mCorrections = new SimpleCorrections(mTabsData);
  addTab(mCorrections, tr("Ctrl+3", "Single Observation tab shortcut"));
#endif
}

SearchWindow::~SearchWindow()
{
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

  if (eq_SensorTime()(mLastNavigated, st))
    return;

#if 1
  BusyIndicator busy;
#else
  // this disables the GUI and is therefore often much slower than updating the views
  BusyStatus busy(this, tr("Preparing data for station %1 at %2, please wait...")
      .arg(st.sensor.stationId)
      .arg(QString::fromStdString(timeutil::to_iso_extended_string(st.time))));
#endif
  mLastNavigated = st;

#ifdef ENABLE_DIANA
  mDianaHelper->navigateTo(st);
#endif
#ifdef ENABLE_ERRORLIST
  mErrorsView->navigateTo(st);
#endif
#ifdef ENABLE_SIMPLECORRECTIONS
  mCorrections->navigateTo(st);
#endif

  mNavigationHistory->navigateTo(st);

  mAutoDataList->navigateTo(st);
  mTimeSeriesView->navigateTo(st);
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
  QSettings settings;
  settings.beginGroup(SETTINGS_SEARCH_GROUP);
  settings.setValue(SETTING_SEARCH_GEOMETRY, saveGeometry());
  settings.setValue(SETTING_SEARCH_AUTOVIEW_SPLITTER, mSplitterDataPlot->saveState());
  settings.setValue(SETTING_SEARCH_DATASEARCH_SPLITTER, splitterDataSearch()->saveState());
  settings.endGroup();

  mErrorsView->saveSettings(settings);
}

void SearchWindow::readSettings()
{
  METLIBS_LOG_SCOPE();

  QSettings settings;
  settings.beginGroup(SETTINGS_SEARCH_GROUP);
  if (not restoreGeometry(settings.value(SETTING_SEARCH_GEOMETRY).toByteArray()))
    METLIBS_LOG_INFO("cannot restore search window geometry");
  if (not mSplitterDataPlot->restoreState(settings.value(SETTING_SEARCH_AUTOVIEW_SPLITTER).toByteArray()))
    METLIBS_LOG_INFO("cannot restore autoview splitter positions");
  if (not splitterDataSearch()->restoreState(settings.value(SETTING_SEARCH_DATASEARCH_SPLITTER).toByteArray()))
    METLIBS_LOG_INFO("cannot restore data-search splitter positions");
  settings.endGroup();

  mErrorsView->restoreSettings(settings);
}

QSplitter* SearchWindow::splitterDataSearch() const
{
  return static_cast<QSplitter*>(centralWidget());
}
