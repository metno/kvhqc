
#include "ExtremesView.hh"

#include "ExtremesTableModel.hh"
#include "common/FindExtremeValues.hh"
#include "common/ParamIdModel.hh"
#include "common/gui/TimeRangeControl.hh"
#include "util/gui/BusyIndicator.hh"

#include <QtGui/QHeaderView>

#include <boost/foreach.hpp>

#include "ui_extremevalues.h"

#define MILOGGER_CATEGORY "kvhqc.ExtremesView"
#include "util/HqcLogging.hh"

ExtremesView::ExtremesView(QWidget* parent)
  : QWidget(parent)
  , ui(new Ui::DialogExtremeValues)
  , mLastSelectedRow(-1)
  , mTimeControl(new TimeRangeControl(this))
{
  METLIBS_LOG_SCOPE();
  ui->setupUi(this);
  ui->tableExtremes->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
  ui->tableExtremes->verticalHeader()->setDefaultSectionSize(20);
  ui->tableExtremes->verticalHeader()->hide();
  ui->tableExtremes->setSelectionBehavior(QTableView::SelectRows);
  ui->tableExtremes->setSelectionMode(QTableView::SingleSelection);

  mTimeControl->setMinimumGap(8);
  mTimeControl->setMaximumGap(24);
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

void ExtremesView::setExtremes(const std::vector<SensorTime>& extremes)
{
  METLIBS_LOG_SCOPE();
  mLastSelectedRow = -1;
  mExtremesModel.reset(new ExtremesTableModel(mEDA, extremes));
  ui->tableExtremes->setModel(mExtremesModel.get());
  ui->tableExtremes->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
  connect(ui->tableExtremes->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
      this, SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));
}

void ExtremesView::onSelectionChanged(const QItemSelection&, const QItemSelection&)
{
  METLIBS_LOG_SCOPE();

  const int row = getSelectedRow();
  if (row < 0 or row == mLastSelectedRow)
    return;
  mLastSelectedRow = row;

  EditDataPtr obs = mExtremesModel->getObs(row);
  Q_EMIT signalNavigateTo(obs->sensorTime());
}

void ExtremesView::onUpdateClicked()
{
  const int paramId = getParamId();
  if (paramId <= 0)
    return;

  {
    BusyIndicator busy;
    const std::vector<SensorTime> extremes = Extremes::find(paramId, mTimeControl->timeRange());
    setExtremes(extremes);
  }
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
