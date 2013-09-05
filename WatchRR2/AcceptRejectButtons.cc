
#include "AcceptRejectButtons.hh"

#include "AcceptReject.hh"
#include "DataColumn.hh"
#include "ObsTableModel.hh"

#include <QtGui/QCheckBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QItemSelection>
#include <QtGui/QTableView>
#include <QtGui/QToolButton>

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.AcceptRejectButtons"
#include "HqcLogging.hh"

AcceptRejectButtons::AcceptRejectButtons(QWidget* parent)
  : QWidget(parent)
  , mTableView(0)
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

  retranslateUi();

  QObject::connect(mButtonAccept, SIGNAL(clicked()), this, SLOT(onAccept()));
  QObject::connect(mButtonReject, SIGNAL(clicked()), this, SLOT(onReject()));
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
    BOOST_FOREACH(SensorTime& st, mSelectedObs) {
      if (mSelectedColumnIsOriginal)
        AcceptReject::accept_original(mDA, st, qc2ok);
      else
        AcceptReject::accept_corrected(mDA, st, qc2ok);
    }
  }
  enableButtons();
}

void AcceptRejectButtons::onReject()
{
  METLIBS_LOG_SCOPE();
  if (not mSelectedObs.empty()) {
    const bool qc2ok = mCheckQC2->isChecked();
    mDA->newVersion();
    BOOST_FOREACH(SensorTime& st, mSelectedObs) {
      AcceptReject::reject(mDA, st, qc2ok);
    }
  }
  enableButtons();
}

void AcceptRejectButtons::updateModel(EditAccessPtr da, QTableView* table)
{
  QObject::disconnect(table->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
      this, SLOT(enableButtons()));

  mDA = da;
  mTableView = table;
  QObject::connect(table->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
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
  if (not selected.isEmpty()) {
    int minRow = selected.at(0).row(), maxRow = minRow;
    int minCol = selected.at(0).column(), maxCol = minCol;
    for (int i=1; i<selected.count(); i++) {
        const int r = selected.at(i).row(), c = selected.at(i).column();
        if (r < minRow)
          minRow = r;
        if (maxRow < r)
          maxRow = r;
        if (c < minCol)
          minCol = c;
        if (maxCol < c)
          maxCol = c;
    }
    if (minCol == maxCol and (maxRow - minRow + 1 == selected.size())) {
      ObsTableModel* tableModel = static_cast<ObsTableModel*>(mTableView->model());
      DataColumnPtr dc = boost::dynamic_pointer_cast<DataColumn>(tableModel->getColumn(minCol));
      if (dc and (dc->type() == ObsColumn::ORIGINAL or dc->type() == ObsColumn::NEW_CORRECTED)) {
        int possible = AcceptReject::ALL;
        for (int r=minRow; r<=maxRow; ++r) {
          const SensorTime st = tableModel->findSensorTime(tableModel->index(r, minCol));
          EditDataPtr obs = mDA->findE(st);
          if (obs) {
            possible &= AcceptReject::possibilities(obs);
            mSelectedObs.push_back(st);
          }
          // TODO disable if missing but in obs_pgm
        }
        if (dc->type() == ObsColumn::ORIGINAL) {
          enableAccept = (possible & AcceptReject::CAN_ACCEPT_ORIGINAL) != 0;
          mSelectedColumnIsOriginal = true;
        } else if (dc->type() == ObsColumn::NEW_CORRECTED) {
          enableAccept = (possible & AcceptReject::CAN_ACCEPT_CORRECTED) != 0;
          mSelectedColumnIsOriginal = false;
        }
        enableReject = (possible & AcceptReject::CAN_REJECT) != 0;
      }
    }
  }
  mButtonAccept->setEnabled(enableAccept);
  mButtonReject->setEnabled(enableReject);
}
