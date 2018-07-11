/* -*- c++ -*-
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

#ifndef HQC_HQCAPPWINDOW_H
#define HQC_HQCAPPWINDOW_H

#include "common/Sensor.hh"

#include <QMainWindow>

#include <memory>

class HelpDialog;
QT_BEGIN_NAMESPACE
class QLabel;
class QTimer;
QT_END_NAMESPACE

class ActionButton;
class ClientSelection;
class DianaHelper;
class EditVersionModel;

class Ui_AppWindow;

class HqcAppWindow : public QMainWindow
{ Q_OBJECT;

public:
  HqcAppWindow();
  ~HqcAppWindow();

  void startup(const QString& captionSuffix);
  void finish();

  DianaHelper* dianaHelper() { return mDianaHelper; }

protected:
  virtual void closeEvent(QCloseEvent* event);
  virtual void changeEvent(QEvent *event);
 
private Q_SLOTS:
  void onNewSearch();
  void onNewWatchRR();
  void onNewRejectedObs();
  void onNewTextdata();

  void onStartKro();
  void onHelpUse();
  void onHelpNews();
  void onHelpFlag();
  void onHelpParam();
  void onAboutHqc();
  void onAboutQt();

  void onUserSettings();
  void kvalobsAvailable(bool);

  void onVersionCheckTimeout();

  void onSaveChanges();
  void onUndoChanges();
  void onRedoChanges();

  void onEditVersionChanged(size_t current, size_t highest);

  void navigateTo(const SensorTime&);

private:
  void writeSettings();
  void readSettings();
  void checkVersionSettings();
  void retranslateUi();
  void showHelpDialog(int doc, const std::string& anchor = std::string());

private:
  std::unique_ptr<Ui_AppWindow> ui;
  QTimer* mVersionCheckTimer;
  HelpDialog* mHelpDialog;
  EditVersionModel* mEditVersions;
  QLabel* mKvalobsAvailable;

  ActionButton *mActionButtonSave;
  ActionButton *mActionButtonUndo;
  ActionButton *mActionButtonRedo;

  ClientSelection* mCoserverClient;
  DianaHelper* mDianaHelper;

  SensorTime mLastNavigated;
  SensorTime mLastNavigatedWatchRR;
};

#endif // HQC_HQCAPPWINDOW_H
