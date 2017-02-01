
#include "ToolInterpolate.hh"

#include "common/DataColumn.hh"
#include "common/KvMetaDataBuffer.hh"
#include "common/ObsHelpers.hh"
#include "common/ObsTableModel.hh"
#include "util/Helpers.hh"
#include "util/make_set.hh"
#include "util/UiHelpers.hh"

#include <QtCore/QEvent>
#include <QtGui/QHBoxLayout>
#include <QtGui/QItemSelection>
#include <QtGui/QTableView>
#include <QtGui/QToolButton>

#define MILOGGER_CATEGORY "kvhqc.ToolInterpolate"
#include "common/ObsLogging.hh"

ToolInterpolate::ToolInterpolate(QWidget* parent)
  : QWidget(parent)
  , mTableView(0)
{
  QHBoxLayout *hLayout = new QHBoxLayout(this);
  hLayout->setSpacing(2);
  hLayout->setContentsMargins(0, 0, 0, 0);
  hLayout->addSpacing(8);

  mButtonInterpolate = new QToolButton(this);
  mButtonInterpolate->setEnabled(false);
  mButtonInterpolate->setIcon(QIcon("icons:interpolate.svg"));
  hLayout->addWidget(mButtonInterpolate);

  retranslateUi();

  QObject::connect(mButtonInterpolate, SIGNAL(clicked()), this, SLOT(onInterpolate()));
}

void ToolInterpolate::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::LanguageChange)
    retranslateUi();
  QWidget::changeEvent(event);
}

void ToolInterpolate::retranslateUi()
{
  mButtonInterpolate->setToolTip(tr("Perform linear interpolation"));
  mButtonInterpolate->setText(tr("Interpolate"));
}

void ToolInterpolate::onInterpolate()
{
  METLIBS_LOG_SCOPE();
  if (not checkEnabled())
    return;

  ObsData_p d0 = mObsBuffer->get(mFirst), d1 = mObsBuffer->get(mLast);
  if (not (d0 and d1))
    return;

  float c0 = d0->corrected(), c1 = d1->corrected();
  float modulo = 0;
  int decimals = 1;
  const bool is_direction = KvMetaDataBuffer::instance()->isDirectionInDegreesParam(d0->sensorTime().sensor.paramId);
  bool direction_start_0 = false, direction_stop_0 = false;
  if (is_direction) {
    decimals = 0;
    modulo = 360;
    direction_start_0 = c0 == 0;
    direction_stop_0  = c1 == 0;
    if (abs(c0+360 - c1) < abs(c0 - c1) and not (direction_start_0 or direction_stop_0))
      c0 += 360;
    METLIBS_LOG_DEBUG(LOGVAL(modulo) << LOGVAL(c0));
  }
  const SensorTime& st0 = d0->sensorTime(), st1 = d1->sensorTime();
  const float dt = (st1.time - st0.time).total_seconds(), dti = 1/dt;
  METLIBS_LOG_DEBUG(LOGVAL(st0) << LOGVAL(st1) << LOGVAL(c0) << LOGVAL(c1) << LOGVAL(dt));

  mDA->newVersion();
  ObsUpdate_pv updates;
  for (Time_v::const_iterator it = mSelectedTimes.begin(); it != mSelectedTimes.end(); ++it) {
    const float tdiff = (*it - st0.time).total_seconds(), r = tdiff * dti;
    float c = Helpers::roundDecimals(r*c1 + (1-r)*c0, decimals);
    if (modulo > 0)
      c = std::fmod(c, modulo);
    if (direction_start_0 or direction_stop_0)
      c = 0;
    else if (is_direction and c == 0)
      c = 360;
    METLIBS_LOG_DEBUG(LOGVAL(*it) << LOGVAL(tdiff) << LOGVAL(r) << LOGVAL(c));

    const SensorTime st(mFirst.sensor, *it);
    ObsData_p obs = mObsBuffer->get(st);
    ObsUpdate_p update = obs ? mDA->createUpdate(obs) : mDA->createUpdate(st);
    Helpers::correct(update, c);
    updates.push_back(update);
  }
  mDA->storeUpdates(updates);
}

void ToolInterpolate::updateModel(EditAccess_p da, QTableView* table)
{
  QObject::disconnect(table->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
      this, SLOT(fetchData()));
  QObject::disconnect(table->model(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
      this, SLOT(enableButtons()));

  mDA = da;
  mTableView = table;

  QObject::connect(table->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
      this, SLOT(fetchData()));
  QObject::connect(table->model(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
      this, SLOT(enableButtons()));

  fetchData();
}

void ToolInterpolate::fetchData()
{
  METLIBS_LOG_SCOPE();

  mButtonInterpolate->setEnabled(false);

  if (not (mTableView and mTableView->selectionModel()))
    return;

  const QModelIndexList selected = mTableView->selectionModel()->selectedIndexes();
  if (selected.size() < 3)
    return;

  int minRow, maxRow, minCol, maxCol;
  Helpers::findMinMaxRowCol(selected, minRow, maxRow, minCol, maxCol);
  if (minCol != maxCol or (maxRow - minRow + 1 != selected.size()))
    return;

  ObsTableModel* tableModel = static_cast<ObsTableModel*>(mTableView->model());
  DataColumn_p dc = std::dynamic_pointer_cast<DataColumn>(tableModel->getColumn(minCol));
  if (not dc or dc->type() != ObsColumn::NEW_CORRECTED)
    return;

  mFirst = tableModel->findSensorTime(tableModel->index(minRow, minCol));
  mLast = tableModel->findSensorTime(tableModel->index(maxRow, minCol));

  mSelectedTimes.clear();
  for (int r=minRow+1; r<maxRow; ++r) {
    const Time t = tableModel->findSensorTime(tableModel->index(r, minCol)).time;
    mSelectedTimes.push_back(t);
  }

  mObsBuffer = std::make_shared<TimeBuffer>(make_set<Sensor_s>(mFirst.sensor), TimeSpan(mFirst.time, mLast.time));
  connect(mObsBuffer.get(), SIGNAL(bufferCompleted(const QString&)),  this, SLOT(enableButtons()));
  connect(mObsBuffer.get(), SIGNAL(newDataEnd(const ObsData_pv&)), this, SLOT(enableButtons()));
  connect(mObsBuffer.get(), SIGNAL(updateDataEnd(const ObsData_pv&)), this, SLOT(enableButtons()));
  connect(mObsBuffer.get(), SIGNAL(dropDataEnd(const SensorTime_v&)), this, SLOT(enableButtons()));
  mObsBuffer->postRequest(mDA);
}

void ToolInterpolate::enableButtons()
{
  METLIBS_LOG_SCOPE();
  mButtonInterpolate->setEnabled(checkEnabled());
}

bool ToolInterpolate::checkEnabled()
{
  METLIBS_LOG_SCOPE();

  if (not mObsBuffer)
    return false;

  ObsData_p o0 = mObsBuffer->get(mFirst);
  if (not o0 or Helpers::is_missing(o0) or Helpers::is_rejected(o0))
    return false;

  ObsData_p o1 = mObsBuffer->get(mLast);
  if (not o1 or Helpers::is_missing(o1) or Helpers::is_rejected(o1))
    return false;

  const bool is_direction = KvMetaDataBuffer::instance()->isDirectionInDegreesParam(mFirst.sensor.paramId);
  if (is_direction) {
    const bool direction_start_0 = (o0->corrected() == 0);
    const bool direction_stop_0  = (o1->corrected() == 0);
    if (direction_start_0 != direction_stop_0)
      return false;
  }

  return true;
}
