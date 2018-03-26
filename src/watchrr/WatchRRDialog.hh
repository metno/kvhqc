/*
  HQC - Free Software for Manual Quality Control of Meteorological Observations

  Copyright (C) 2018 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of HQC

  HQC is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  HQC is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
  for more details.

  You should have received a copy of the GNU General Public License
  along with HQC; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#ifndef WATCHRRDIALOG_HH
#define WATCHRRDIALOG_HH 1

#include "TaskAccess.hh"
#include "common/ModelAccess.hh"
#include "common/TimeSpan.hh"

#include <QDialog>

#include <memory>

class HqcAppWindow;

QT_BEGIN_NAMESPACE
class QItemSelection;
class QModelIndex;
QT_END_NAMESPACE

namespace Ui {
class DialogMain;
}
class StationCardModel;
class NeighborRR24Model;
class NeighborCardsModel;
class WatchRRDianaClient;

class WatchRRDialog : public QDialog
{   Q_OBJECT;
public:
  WatchRRDialog(EditAccess_p da, ModelAccess_p ma, const Sensor& sensor, const TimeSpan& time, HqcAppWindow* parent=0);
  ~WatchRRDialog();

public Q_SLOTS:
  virtual void accept();
  virtual void reject();
  
protected:
  virtual void changeEvent(QEvent *event);

private Q_SLOTS:
  void onAcceptRow();
  void onEdit();
  void onRedistribute();
  void onRedistributeQC2();
  void onUndo();
  void onRedo();
  void onSelectionChanged(const QItemSelection&, const QItemSelection&);
  void onNeighborDataDateChanged(const QDate&);
  void onNeighborDataTimeChanged(const timeutil::ptime& time);
  void onNeighborDataTimeChanged(const SensorTime& st);
  void onCurrentTabChanged(int tab);

private:
  struct Selection {
    TimeSpan selTime;
    int minCol;
    int maxCol;
    Selection()
      : minCol(-1), maxCol(-1) { }
    Selection(const TimeSpan& s, int mic, int mac)
      : selTime(s), minCol(mic), maxCol(mac) { }
    bool empty() const
      { return minCol<0 or maxCol<0; }
  };

private:
  void setStationInfoText();
  Selection findSelection();
  void clearSelection();
  enum RR24SelectionType { NO_RR24 = 0, RR24_CORRECTED, RR24_ORIGINAL };
  RR24SelectionType isRR24Selection(const Selection& sel) const;
  bool isCompleteSingleRowSelection(const Selection& sel) const;

  void initializeRR24Data();
  void addRR24Task(const timeutil::ptime& time, QString task);
  void enableSave();

private:
  std::unique_ptr<Ui::DialogMain> ui;
  std::unique_ptr<WatchRRDianaClient> mDianaClient;
  EditAccess_p mParentDA;
  TaskAccess_p mDA;
  Sensor mSensor;
  TimeSpan mTime;
  TimeSpan mEditableTime;
  std::unique_ptr<StationCardModel> mStationCard;
  std::unique_ptr<NeighborRR24Model> mNeighborRR24;
  std::unique_ptr<NeighborCardsModel> mNeighborCards;
};

#endif // WATCHRRDIALOG_HH
