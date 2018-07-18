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


#include "AcceptRejectButtons.hh"

#include "common/AcceptReject.hh"
#include "common/DataColumn.hh"
#include "common/ModelColumn.hh"
#include "common/ObsTableModel.hh"
#include "common/SingleObsBuffer.hh"
#include "util/UiHelpers.hh"

#include <QCheckBox>
#include <QEvent>
#include <QHBoxLayout>
#include <QItemSelection>
#include <QTableView>
#include <QToolButton>

#define MILOGGER_CATEGORY "kvhqc.AcceptRejectButtons"
#include "util/HqcLogging.hh"

AcceptRejectButtons::AcceptRejectButtons(QWidget* parent)
  : QWidget(parent)
  , mTableView(0)
  , mSelectedColumnType(OTHER)
{
  QHBoxLayout *hLayout = new QHBoxLayout(this);
  hLayout->setSpacing(2);
  hLayout->setContentsMargins(0, 0, 0, 0);

  mButtonAccept = new QToolButton(this);
  mButtonAccept->setEnabled(false);
  mButtonAccept->setIcon(QIcon("icons:accept.svg"));
  hLayout->addWidget(mButtonAccept);

  mButtonReject = new QToolButton(this);
  mButtonReject->setEnabled(false);
  mButtonReject->setIcon(QIcon("icons:reject.svg"));
  hLayout->addWidget(mButtonReject);

  mCheckQC2 = new QCheckBox(this);
  hLayout->addWidget(mCheckQC2);
  hLayout->addSpacing(8);

  retranslateUi();

  QObject::connect(mButtonAccept, SIGNAL(clicked()), this, SLOT(onAccept()));
  QObject::connect(mButtonReject, SIGNAL(clicked()), this, SLOT(onReject()));
  QObject::connect(mCheckQC2, SIGNAL(toggled(bool)), this, SLOT(enableButtons()));
}

void AcceptRejectButtons::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::LanguageChange)
    retranslateUi();
  QWidget::changeEvent(event);
}

void AcceptRejectButtons::retranslateUi()
{
  mButtonAccept->setToolTip(tr("<html><head/><body><p>Accept the original or corrected value"
          " (depending on the selected column).</p><p>QC2 is allowed to override if "
          "<span style=\" font-style:italic;\">QC2</span> is checked.</p></body></html>"));
  mButtonAccept->setText(tr("Accept"));
  mButtonReject->setToolTip(tr("<html><head/><body><p>Reject the value.</p><p>QC2 is allowed"
          " to override if <span style=\" font-style:italic;\">QC2</span>"
          " is checked.</p></body></html>"));
  mButtonReject->setText(tr("Reject"));
  mCheckQC2->setToolTip(tr("<html><head/><body><p>Decides wheter QC2 is allowed to override"
          " for <span style=\" font-style:italic;\">Accept</span> and "
          "<span style=\"font-style:italic;\">Reject</span>.</p></body></html>"));
  mCheckQC2->setText(tr("QC2"));
}

void AcceptRejectButtons::onAccept()
{
  METLIBS_LOG_SCOPE();
  if (!mSelectedObs.empty()) {
    disableDataChanged();
    const bool qc2ok = mCheckQC2->isChecked();
    mDA->newVersion();
    if (mSelectedColumnType == ORIGINAL and not qc2ok) {
      AcceptReject::accept_original(mDA, mSelectedObs);
    } else if (mSelectedColumnType == CORRECTED) {
      AcceptReject::accept_corrected(mDA, mSelectedObs, qc2ok);
    } else if (mSelectedColumnType == MODEL and mMA) {
      AcceptReject::accept_model(mDA, mMA, mSelectedObs, qc2ok);
    }
    enableDataChanged();
  }
}

void AcceptRejectButtons::onReject()
{
  METLIBS_LOG_SCOPE();
  if (not mSelectedObs.empty() and (mSelectedColumnType == ORIGINAL or mSelectedColumnType == CORRECTED)) {
    const bool qc2ok = mCheckQC2->isChecked();
    disableDataChanged();
    mDA->newVersion();
    AcceptReject::reject(mDA, mSelectedObs, qc2ok);
    enableDataChanged();
  }
}

void AcceptRejectButtons::disableDataChanged()
{
  if (mTableView) {
    QObject::disconnect(mTableView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(enableButtons()));
    QObject::disconnect(mTableView->model(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(enableButtons()));
  }
}

void AcceptRejectButtons::enableDataChanged()
{
  if (mTableView) {
    QObject::connect(mTableView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(enableButtons()));
    QObject::connect(mTableView->model(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(enableButtons()));
  }
  enableButtons();
}

void AcceptRejectButtons::updateModel(EditAccess_p da, ModelAccess_p ma, QTableView* table)
{
  disableDataChanged();

  mDA = da;
  mMA = ma;
  mTableView = table;

  enableDataChanged();
}

void AcceptRejectButtons::enableButtons()
{
  METLIBS_LOG_SCOPE();
  bool enableAccept = false, enableReject = false;
  mSelectedObs.clear();
  mSelectedColumnType = NONE;

  if (mTableView && mTableView->selectionModel()) {
    int possible = AcceptReject::ALL;
    ObsTableModel* tableModel = static_cast<ObsTableModel*>(mTableView->model());
    typedef std::set<SensorTime, lt_SensorTime> SensorTime_s;
    SensorTime_s sensortimes;
    Sensor_s sensors;
    TimeSpan timespan;
    for (const QModelIndex& sel : mTableView->selectionModel()->selectedIndexes()) {
      SelectedColumnType ct = OTHER;
      if (DataColumn_p dc = std::dynamic_pointer_cast<DataColumn>(tableModel->getColumn(sel.column()))) {
        if (dc->type() == ObsColumn::ORIGINAL)
          ct = ORIGINAL;
        else if (dc->type() == ObsColumn::NEW_CORRECTED)
          ct = CORRECTED;
      } else if (ModelColumn_p mc = std::dynamic_pointer_cast<ModelColumn>(tableModel->getColumn(sel.column()))) {
        if (mMA)
          ct = MODEL;
      }
      if (mSelectedColumnType == NONE)
        mSelectedColumnType = ct;
      else if (mSelectedColumnType != ct)
        mSelectedColumnType = OTHER;
      if (mSelectedColumnType == OTHER) {
        sensortimes.clear();
        possible = 0;
        break;
      }
      const SensorTime st = tableModel->findSensorTime(sel);
      sensortimes.insert(st);
      sensors.insert(st.sensor);
      if (timespan.t0().is_not_a_date_time() || timespan.t0() > st.time)
        timespan.t0() = st.time;
      if (timespan.t1().is_not_a_date_time() || timespan.t1() < st.time)
        timespan.t1() = st.time;
    }
    if (timespan.closed() && !sensortimes.empty()) {
      TimeBuffer_p buffer = std::make_shared<TimeBuffer>(sensors, timespan);
      buffer->syncRequest(mDA);
      for (const SensorTime& st : sensortimes) {
        if (ObsData_p obs = buffer->get(st)) {
          possible &= AcceptReject::possibilities(obs);
          mSelectedObs.push_back(obs);
        }
      }
      // TODO disable if missing but in obs_pgm
    }
    METLIBS_LOG_DEBUG(LOGVAL(mSelectedColumnType) << LOGVAL(possible));
    enableReject = (possible & AcceptReject::CAN_REJECT) != 0;
    if (mSelectedColumnType == ORIGINAL) {
      if (mCheckQC2->isChecked())
        enableAccept = false;
      else
        enableAccept = (possible & AcceptReject::CAN_ACCEPT_ORIGINAL) != 0;
    } else if (mSelectedColumnType == CORRECTED) {
      enableAccept = (possible & AcceptReject::CAN_ACCEPT_CORRECTED) != 0;
    } else if (mSelectedColumnType == MODEL) {
      enableAccept = (possible & AcceptReject::CAN_ACCEPT_CORRECTED) != 0;
      enableReject = false;
    } else {
      // nothing allowed for other columns, or mixed columns
      enableAccept = enableReject = false;
    }
  }

  mButtonAccept->setEnabled(enableAccept);
  mButtonReject->setEnabled(enableReject);
}
