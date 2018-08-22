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


#include "DataList.hh"

#include "common/DataListModel.hh"
#include "common/ObsDelegate.hh"
#include "common/HqcApplication.hh"
#include "util/Helpers.hh"

#include <QCheckBox>
#include <QFile>
#include <QFileDialog>
#include <QFont>
#include <QHeaderView>
#include <QPushButton>
#include <QTextStream>

#include <set>

#include "ui_datalist.h"

#define MILOGGER_CATEGORY "kvhqc.DataList"
#include "common/ObsLogging.hh"

namespace /* anonymous */ {
QString protectForCSV(QString txt)
{
  txt.replace('\n', " ");
  txt.replace('\"', "'");
  txt.replace('\t', " ");
  return txt;
}
QString protectForCSV(const QVariant& v)
{
  return protectForCSV(v.toString());
}
const int MINUTE = 60, HOUR = 60*MINUTE;
} // namespace anonymous

DataList::DataList(QWidget* parent)
  : DynamicDataView(parent)
  , ui(new Ui_DataList)
  , mDA(hqcApp->editAccess())
  , mMA(hqcApp->modelAccess())
  , mCurrentSelected(false)
{
  ui->setupUi(this);
  ui->buttonSaveAs->setIcon(QIcon("icons:dl_save_as.svg"));

  mBusy = new BusyLabel(this);
  ui->layoutButtons->addWidget(mBusy);

  QFont mono("Monospace");
  ui->table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  ui->table->horizontalHeader()->setSectionsMovable(true);
  ui->table->verticalHeader()->setFont(mono);
  ui->table->verticalHeader()->setDefaultSectionSize(20);

  ui->table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  ui->table->setSelectionBehavior(QAbstractItemView::SelectItems);
  ui->table->setSelectionMode(QAbstractItemView::ExtendedSelection);
  ui->table->setItemDelegate(new ObsDelegate(this));
  
  // need to do this before adding time steps
  DataListModel* mdl = new DataListModel(mDA, this);
  ui->table->setModel(mdl);

  ui->comboTimeFilter->addItem(tr("none"), QVariant(0));
  { const int NMINUTES = 5, MINUTES[NMINUTES] = { 1, 5, 10, 15, 30 };
    for (int i=0; i<NMINUTES; ++i)
      addTimeStepItem(MINUTES[i]*MINUTE);
  }

  { const int NHOURS = 5, HOURS[NHOURS] = { 1, 3, 6, 12, 24 };
    for (int i=0; i<NHOURS; ++i)
      addTimeStepItem(HOURS[i]*HOUR);
  }
  ui->checkTimeFilter->setEnabled(false);

  onUITimeStepChanged(ui->comboTimeFilter->currentIndex());

  connect(mdl, SIGNAL(changedTimeStep(int)),
      this, SLOT(onModelTimeStepChanged(int)));
  connect(mdl, SIGNAL(changedFilterByTimestep(bool, bool)),
      this, SLOT(onModelFilterByTimeStepChanged(bool, bool)));
  connect(mdl, SIGNAL(busyStatus(bool)),
      this, SLOT(onBusyStatus(bool)));
  
  ui->buttonsAcceptReject->updateModel(mDA, mMA, ui->table);
  ui->toolInterpolate->updateModel(mDA, ui->table);

  connect(ui->table->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
      this, SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));
  connect(ui->table, SIGNAL(signalCurrentChanged(const QModelIndex&)),
      this, SLOT(onCurrentChanged(const QModelIndex&)));
}

DataList::~DataList()
{
}

DataListModel* DataList::model() const
{
  return static_cast<DataListModel*>(ui->table->model());
}

void DataList::addTimeStepItem(int step)
{
  ui->comboTimeFilter->addItem(Helpers::timeStepAsText(step), QVariant(step));
}

void DataList::retranslateUi()
{
  ui->retranslateUi(this);
  for (int i = 0; i < ui->comboTimeFilter->count(); ++i) {
    const int step = ui->comboTimeFilter->itemData(i).toInt();
    ui->comboTimeFilter->setItemText(i, Helpers::timeStepAsText(step));
  }
  VisibleWidget::retranslateUi();
}

void DataList::doNavigateTo()
{
  DynamicDataView::doNavigateTo();
  reselectCurrent();
}

void DataList::reselectCurrent()
{
  mCurrentSelected = false;
  selectCurrent();
}

void DataList::selectCurrent()
{
  METLIBS_LOG_SCOPE(LOGMYTYPE() << LOGVAL(mCurrentSelected));
  if (mCurrentSelected)
    return;

  const QModelIndexList idxs = model()->findIndexes(mNavigate.current());
  METLIBS_LOG_DEBUG(LOGVAL(idxs.isEmpty()));
  if (idxs.isEmpty())
    return;

  METLIBS_LOG_DEBUG("found " << idxs.size() << " indexes for " << mNavigate.current());
  mCurrentSelected = true;

  const QModelIndex& currentIdx = ui->table->currentIndex();
  QItemSelection selection;
  bool scroll = (not idxs.empty());
  for (const QModelIndex& idx : idxs) {
    METLIBS_LOG_DEBUG("  -- r=" << idx.row() << " c=" << idx.column());
    selection.select(idx, idx);
    if (idx == currentIdx)
      scroll = false;
  }
  // IMPORTANT scroll after select does not work
  if (scroll) {
    ui->table->scrollTo(idxs.front());
    ui->table->scrollTo(idxs.back());
  }
  if (QItemSelectionModel* sm = ui->table->selectionModel()) {
    sm->select(selection, QItemSelectionModel::ClearAndSelect);
    if (scroll)
      sm->setCurrentIndex(idxs.back(), QItemSelectionModel::NoUpdate);
  }
}

void DataList::onCurrentChanged(const QModelIndex& current)
{
  METLIBS_LOG_SCOPE(LOGMYTYPE() << LOGVAL(current.isValid()));
  if (current.isValid()) {
    mCurrentSelected = true;
    sendNavigateTo(model()->findSensorTime(current));
  }
}

void DataList::onSelectionChanged(const QItemSelection&, const QItemSelection&)
{
}

void DataList::onBusyStatus(bool busy)
{
  METLIBS_LOG_SCOPE(LOGMYTYPE() << LOGVAL(busy));
  mBusy->setBusy(busy);
  if (not busy)
    reselectCurrent();
}

void DataList::onCheckFilter(bool filterByTimestep)
{
  model()->setFilterByTimestep(filterByTimestep);
}

void DataList::onUITimeStepChanged(int index)
{
  int step = 0;
  if (index >= 0)
    step = ui->comboTimeFilter->itemData(index).toInt();
  model()->setTimeStep(step);
}

void DataList::onModelTimeStepChanged(int step)
{
  for (int i = 0; i < ui->comboTimeFilter->count(); ++i) {
    const int combostep = ui->comboTimeFilter->itemData(i).toInt();
    if (step == combostep) {
      ui->comboTimeFilter->setCurrentIndex(i);
      return;
    }
  }
  METLIBS_LOG_WARN("datalist model time step '" << step << "' not in combo list");
  if (step > 0) {
    addTimeStepItem(step);
    ui->comboTimeFilter->setCurrentIndex(ui->comboTimeFilter->count() - 1);
  }
}

void DataList::onModelFilterByTimeStepChanged(bool enabled, bool ftbs)
{
  ui->checkTimeFilter->setEnabled(enabled);
  ui->checkTimeFilter->setChecked(ftbs);
}

void DataList::onButtonSaveAs()
{
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save Table"),
      "datalist.csv",
      tr("CSV Table (*.csv)"));
  if (fileName.isEmpty())
    return;
  
  QFile file(fileName);
  file.open(QIODevice::WriteOnly | QIODevice::Text);

  int r0, r1, c0, c1;

  std::set<int> selectedRows, selectedColumns;
  std::set< std::pair<int,int> > selectedCells;
  QModelIndexList selected;
  if (QItemSelectionModel* sm = ui->table->selectionModel())
    selected = sm->selectedIndexes();
  if (not selected.isEmpty()) {
    for (int i=0; i<selected.count(); i++) {
      const int r = selected.at(i).row(), c = selected.at(i).column();
      selectedRows.insert(r);
      selectedColumns.insert(c);
      selectedCells.insert(std::make_pair(r, c));
    }
    r0 = *selectedRows.begin();
    r1 = *selectedRows.rbegin();
    c0 = *selectedColumns.begin();
    c1 = *selectedColumns.rbegin();
  } else {
    r0 = 0;
    r1 = model()->rowCount(QModelIndex())-1; // no problem if < 0
    c0 = 0;
    c1 = model()->columnCount(QModelIndex())-1; // no problem if < 0
  }

  QTextStream out(&file);

  out << "\"\"";
  for (int c=c0; c<=c1; ++c) {
    if (selectedColumns.empty() or selectedColumns.find(c) != selectedColumns.end())
      out << "\t\"" << protectForCSV(model()->headerData(c, Qt::Horizontal, Qt::ToolTipRole)) << '\"';
  }
  out << "\n";

  for (int r=r0; r<=r1; ++r) {
    if (selectedRows.empty() or selectedRows.find(r) != selectedRows.end()) {
      out << '\"' << protectForCSV(model()->headerData(r, Qt::Vertical, Qt::DisplayRole)) << '\"';
      for (int c=c0; c<=c1; ++c) {
        if (selectedColumns.empty() or selectedColumns.find(c) != selectedColumns.end()) {
          QString cell;
          if (selectedCells.empty() or selectedCells.find(std::make_pair(r, c)) != selectedCells.end())
            cell = protectForCSV(model()->data(model()->index(r, c), Qt::DisplayRole));
          out << "\t\"" << cell << '\"';
        }
      }
      out << "\n";
    }
  }
 
  file.close(); 
}
