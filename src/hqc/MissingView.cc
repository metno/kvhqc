
#include "MissingView.hh"

#include "MissingTableModel.hh"
#include "common/FindMissingValues.hh"
#include "common/TypeIdModel.hh"
#include "common/gui/TimeRangeControl.hh"
#include "util/gui/BusyIndicator.hh"

#include <QtGui/QHeaderView>

#include <boost/foreach.hpp>

#include "ui_missingvalues.h"

#define MILOGGER_CATEGORY "kvhqc.MissingView"
#include "util/HqcLogging.hh"

MissingView::MissingView(QWidget* parent)
  : QWidget(parent)
  , ui(new Ui::DialogMissingObservations)
  , mLastSelectedRow(-1)
  , mTimeControl(new TimeRangeControl(this))
{
  METLIBS_LOG_SCOPE();
  ui->setupUi(this);
  ui->tableMissing->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
  ui->tableMissing->verticalHeader()->setDefaultSectionSize(20);
  ui->tableMissing->verticalHeader()->hide();
  ui->tableMissing->setSelectionBehavior(QTableView::SelectRows);
  ui->tableMissing->setSelectionMode(QTableView::SingleSelection);

  mTimeControl->setMinimumGap(  24);
  mTimeControl->setMaximumGap(3*24);
  mTimeControl->install(ui->timeFrom, ui->timeTo);

  timeutil::ptime now = timeutil::now();
  int hour = now.time_of_day().hours();
  if (hour < 6)
    now -= boost::gregorian::days(1);
  const timeutil::ptime t1 = timeutil::from_YMDhms(now.date().year(), now.date().month(), now.date().day(), 0, 0, 0);
  const timeutil::ptime t0 = t1 - boost::gregorian::days(1);

  ui->timeFrom->setDateTime(timeutil::to_QDateTime(t0));
  ui->timeTo  ->setDateTime(timeutil::to_QDateTime(t1));

  std::vector<int> types;
  types.push_back(  0); // all
  types.push_back(  3); // autoobs
  types.push_back(302); // sms2
  types.push_back(308); // sms8
  types.push_back(312); // sms12
  types.push_back(402); // ukekort
  types.push_back(412); // dagbok
  
  OTypeIdExtract typeExtract;
  typeExtract.override(0, tr("any"));
  ui->comboType->setModel(new OverrideTypeIdModel(types, typeExtract));

  ui->comboType->setCurrentIndex(0);
}

MissingView::~MissingView()
{
}

void MissingView::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::LanguageChange)
    ui->retranslateUi(this);
  QWidget::changeEvent(event);
}

void MissingView::setMissing(const std::vector<SensorTime>& missing)
{
  METLIBS_LOG_SCOPE();
  mLastSelectedRow = -1;
  mMissingModel.reset(new MissingTableModel(mEDA, missing));
  ui->tableMissing->setModel(mMissingModel.get());
  ui->tableMissing->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
  ui->tableMissing->horizontalHeader()->resizeSection(MissingTableModel::COL_STATION_ID, 60);
  connect(ui->tableMissing->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
      this, SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));
}

void MissingView::onSelectionChanged(const QItemSelection&, const QItemSelection&)
{
  METLIBS_LOG_SCOPE();

  const int row = getSelectedRow();
  if (row < 0 or row == mLastSelectedRow)
    return;
  mLastSelectedRow = row;

  /*emit*/ signalNavigateTo(mMissingModel->getSensorTime(row));
}

void MissingView::onUpdateClicked()
{
  const int typeId = getTypeId();
  if (typeId < 0)
    return;

  {
    BusyIndicator busy;
    const std::vector<SensorTime> missing = Missing::find(typeId, mTimeControl->timeRange());
    setMissing(missing);
  }
}

int MissingView::getSelectedRow() const
{
  QModelIndexList selectedRows = ui->tableMissing->selectionModel()->selectedRows();
  if (selectedRows.size() != 1)
    return -1;
  const QModelIndex indexModel = selectedRows.at(0);
  return indexModel.row();
}

int MissingView::getTypeId() const
{
  // FIXME same as in DataListAddColumn

  const int idx = ui->comboType->currentIndex();
  if (idx < 0)
      return -1;
  OverrideTypeIdModel* tim = static_cast<OverrideTypeIdModel*>(ui->comboType->model());
  return tim->values().at(idx);
}
