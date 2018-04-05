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

#include "ExtremesView.hh"

#include "ExtremesTableModel.hh"

#include "common/HqcApplication.hh"
#include "common/KvHelpers.hh"
#include "common/ParamIdModel.hh"
#include "common/TimeSpanControl.hh"

#include <QHeaderView>

#include "ui_extremevalues.h"

#define MILOGGER_CATEGORY "kvhqc.ExtremesView"
#include "util/HqcLogging.hh"

ExtremesView::ExtremesView(QWidget* parent)
  : QWidget(parent)
  , ui(new Ui::DialogExtremeValues)
  , mLastSelectedRow(-1)
  , mTimeControl(new TimeSpanControl(this))
{
  METLIBS_LOG_SCOPE();
  ui->setupUi(this);

  mBusy = new BusyLabel(this);
  ui->topRow->addWidget(mBusy);

  ui->tableExtremes->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
  ui->tableExtremes->verticalHeader()->setDefaultSectionSize(20);
  ui->tableExtremes->verticalHeader()->hide();
  ui->tableExtremes->setSelectionBehavior(QTableView::SelectRows);
  ui->tableExtremes->setSelectionMode(QTableView::SingleSelection);

  mTimeControl->setMinimumGap(8);
  mTimeControl->setMaximumGap(4 * 24);
  mTimeControl->install(ui->timeFrom, ui->timeTo);

  timeutil::ptime now = timeutil::now();
  int hour = now.time_of_day().hours();
  if (hour < 6)
    now -= boost::gregorian::days(1);
  const timeutil::ptime t1 = timeutil::from_YMDhms(now.date().year(), now.date().month(), now.date().day(), 6, 0, 0);
  const timeutil::ptime t0 = t1 - boost::gregorian::days(1);

  ui->timeFrom->setDateTime(timeutil::to_QDateTime(t0));
  ui->timeTo  ->setDateTime(timeutil::to_QDateTime(t1));

  std::vector<int> params;
  params.push_back(kvalobs::PARAMID_TAN);
  params.push_back(kvalobs::PARAMID_TAX);
  params.push_back(kvalobs::PARAMID_RR_12);
  params.push_back(kvalobs::PARAMID_RR_24);
  params.push_back(kvalobs::PARAMID_SA);
  params.push_back(kvalobs::PARAMID_FG);
  params.push_back(kvalobs::PARAMID_FX);
  ui->comboParam->setModel(new ParamIdModel(params));
  ui->comboParam->setCurrentIndex(0);

  mExtremesModel.reset(new ExtremesTableModel(hqcApp->editAccess()));
  ui->tableExtremes->setModel(mExtremesModel.get());
  connect(ui->tableExtremes->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
      this, SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));
  connect(mExtremesModel.get(), SIGNAL(modelReset()), this, SLOT(onModelReset()));
}

ExtremesView::~ExtremesView()
{
}

void ExtremesView::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::LanguageChange)
    ui->retranslateUi(this);
  QWidget::changeEvent(event);
}

void ExtremesView::onSelectionChanged(const QItemSelection&, const QItemSelection&)
{
  METLIBS_LOG_SCOPE();

  const int row = getSelectedRow();
  if (row < 0 or row == mLastSelectedRow)
    return;
  mLastSelectedRow = row;

  ObsData_p obs = mExtremesModel->getObs(row);
  Q_EMIT signalNavigateTo(obs->sensorTime());
}

void ExtremesView::onUpdateClicked()
{
  METLIBS_LOG_SCOPE();

  const int paramId = getParamId();
  if (paramId <= 0)
    return;

  mLastSelectedRow = -1;
  mBusy->setBusy(true);
  mExtremesModel->search(paramId, mTimeControl->timeRange());
}

int ExtremesView::getSelectedRow() const
{
  QModelIndexList selectedRows = ui->tableExtremes->selectionModel()->selectedRows();
  if (selectedRows.size() != 1)
    return -1;
  const QModelIndex indexModel = selectedRows.at(0);
  return indexModel.row();
}

int ExtremesView::getParamId() const
{
  // FIXME same as in DataListAddColumn

  const int idx = ui->comboParam->currentIndex();
  if (idx < 0)
      return -1;
  ParamIdModel* pim = static_cast<ParamIdModel*>(ui->comboParam->model());
  return pim->values().at(idx);
}

void ExtremesView::onModelReset()
{
  mBusy->setBusy(false);
  ui->tableExtremes->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}
