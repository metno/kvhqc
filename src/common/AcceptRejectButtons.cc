
#include "AcceptRejectButtons.hh"

#include "common/AcceptReject.hh"
#include "common/DataColumn.hh"
#include "common/ModelColumn.hh"
#include "common/ObsTableModel.hh"
#include "common/SingleObsBuffer.hh"
#include "util/UiHelpers.hh"

#include <QtCore/QEvent>
#include <QtGui/QCheckBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QItemSelection>
#include <QtGui/QTableView>
#include <QtGui/QToolButton>

#include <boost/foreach.hpp>

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
  if (not mSelectedObs.empty()) {
    const bool qc2ok = mCheckQC2->isChecked();
    mDA->newVersion();
    BOOST_FOREACH(ObsData_p& obs, mSelectedObs) {
      if (mSelectedColumnType == ORIGINAL and not qc2ok) {
        AcceptReject::accept_original(mDA, obs);
      } else if (mSelectedColumnType == CORRECTED) {
        AcceptReject::accept_corrected(mDA, obs, qc2ok);
      } else if (mSelectedColumnType == MODEL and mMA) {
        AcceptReject::accept_model(mDA, mMA, obs, qc2ok);
      }
    }
  }
  enableButtons();
}

void AcceptRejectButtons::onReject()
{
  METLIBS_LOG_SCOPE();
  if (not mSelectedObs.empty() and (mSelectedColumnType == ORIGINAL or mSelectedColumnType == CORRECTED)) {
    const bool qc2ok = mCheckQC2->isChecked();
    mDA->newVersion();
    BOOST_FOREACH(ObsData_p& obs, mSelectedObs) {
      AcceptReject::reject(mDA, obs, qc2ok);
    }
  }
  enableButtons();
}

void AcceptRejectButtons::updateModel(EditAccess_p da, ModelAccess_p ma, QTableView* table)
{
  QObject::disconnect(table->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
      this, SLOT(enableButtons()));
  QObject::disconnect(table->model(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
      this, SLOT(enableButtons()));

  mDA = da;
  mMA = ma;
  mTableView = table;

  QObject::connect(table->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
      this, SLOT(enableButtons()));
  QObject::connect(table->model(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
      this, SLOT(enableButtons()));
}

void AcceptRejectButtons::enableButtons()
{
  METLIBS_LOG_SCOPE();
  if (not (mTableView and mTableView->selectionModel())) {
    mButtonAccept->setEnabled(false);
    mButtonReject->setEnabled(false);
    return;
  }

  const QModelIndexList selected = mTableView->selectionModel()->selectedIndexes();
  bool enableAccept = false, enableReject = false;
  mSelectedObs.clear();
  mSelectedColumnType = OTHER;
  if (not selected.isEmpty()) {
    int minRow, maxRow, minCol, maxCol;
    Helpers::findMinMaxRowCol(selected, minRow, maxRow, minCol, maxCol);
    if (minCol == maxCol and (maxRow - minRow + 1 == selected.size())) {
      ObsTableModel* tableModel = static_cast<ObsTableModel*>(mTableView->model());
      if (DataColumn_p dc = std::dynamic_pointer_cast<DataColumn>(tableModel->getColumn(minCol))) {
        if (dc->type() == ObsColumn::ORIGINAL)
          mSelectedColumnType = ORIGINAL;
        else if (dc->type() == ObsColumn::NEW_CORRECTED)
          mSelectedColumnType = CORRECTED;
      } else if (ModelColumn_p mc = std::dynamic_pointer_cast<ModelColumn>(tableModel->getColumn(minCol))) {
        if (mMA)
          mSelectedColumnType = MODEL;
      }
      METLIBS_LOG_DEBUG(LOGVAL(mSelectedColumnType));
      if (mSelectedColumnType != OTHER) {
        int possible = AcceptReject::ALL;
        if (mSelectedColumnType == MODEL)
          possible &= ~AcceptReject::CAN_REJECT;
        for (int r=minRow; r<=maxRow; ++r) {
          const SensorTime st = tableModel->findSensorTime(tableModel->index(r, minCol));
          SingleObsBuffer_p sobs(new SingleObsBuffer(st));
          sobs->syncRequest(mDA);
          ObsData_p obs = sobs->get();
          if (obs) {
            possible &= AcceptReject::possibilities(obs);
            mSelectedObs.push_back(obs);
          }
          // TODO disable if missing but in obs_pgm
        }
        METLIBS_LOG_DEBUG(LOGVAL(possible));
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
        }
      } else {
        // nothing allowed if not CORRECTED, ORIGINAL, or MODEL column (e.g. FLAGS)
        enableAccept = enableReject = false;
      }
    }
  }
  mButtonAccept->setEnabled(enableAccept);
  mButtonReject->setEnabled(enableReject);
}
