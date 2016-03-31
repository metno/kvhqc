
#include "ToolInterpolate.hh"

#include "common/DataColumn.hh"
#include "common/KvMetaDataBuffer.hh"
#include "common/ObsHelpers.hh"
#include "common/ObsTableModel.hh"
#include "util/Helpers.hh"
#include "util/gui/UiHelpers.hh"

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

  const SensorTime &st0 = mSelectedStart, &st1 = mSelectedEnd;
  EditDataPtr d0 = mDA->findE(st0), d1 = mDA->findE(st1);
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
  const float dt = (st1.time - st0.time).total_seconds(), dti = 1/dt;
  METLIBS_LOG_DEBUG(LOGVAL(st0) << LOGVAL(st1) << LOGVAL(c0) << LOGVAL(c1) << LOGVAL(dt));

  mDA->newVersion();
  for (SensorTime_v::const_iterator it = mSelectedObs.begin(); it != mSelectedObs.end(); ++it) {
    const float tdiff = (it->time - st0.time).total_seconds(), r = tdiff * dti;
    float c = Helpers::roundDecimals(r*c1 + (1-r)*c0, decimals);
    if (modulo > 0)
      c = std::fmod(c, modulo);
    if (direction_start_0 or direction_stop_0)
      c = 0;
    else if (is_direction and c == 0)
      c = 360;
    METLIBS_LOG_DEBUG(LOGVAL(*it) << LOGVAL(tdiff) << LOGVAL(r) << LOGVAL(c));

    EditDataPtr obs = mDA->findOrCreateE(*it);
    EditDataEditorPtr editor = mDA->editor(obs);
    Helpers::correct(editor, c);
    editor->commit();
  }
}

void ToolInterpolate::updateModel(EditAccessPtr da, QTableView* table)
{
  QObject::disconnect(table->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
      this, SLOT(enableButtons()));
  QObject::disconnect(table->model(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
      this, SLOT(enableButtons()));

  mDA = da;
  mTableView = table;

  QObject::connect(table->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
      this, SLOT(enableButtons()));
  QObject::connect(table->model(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
      this, SLOT(enableButtons()));
}

void ToolInterpolate::enableButtons()
{
  METLIBS_LOG_SCOPE();
  mButtonInterpolate->setEnabled(checkEnabled());
}

bool ToolInterpolate::checkEnabled()
{
  METLIBS_LOG_SCOPE();
  if (not (mTableView and mTableView->selectionModel()))
    return false;

  const QModelIndexList selected = mTableView->selectionModel()->selectedIndexes();
  if (selected.size() < 3)
    return false;

  int minRow, maxRow, minCol, maxCol;
  Helpers::findMinMaxRowCol(selected, minRow, maxRow, minCol, maxCol);
  if (minCol != maxCol or (maxRow - minRow + 1 != selected.size()))
    return false;

  ObsTableModel* tableModel = static_cast<ObsTableModel*>(mTableView->model());
  DataColumnPtr dc = std::dynamic_pointer_cast<DataColumn>(tableModel->getColumn(minCol));
  if (not dc or dc->type() != ObsColumn::NEW_CORRECTED)
    return false;

  const SensorTime st = tableModel->findSensorTime(tableModel->index(minRow, minCol));
  const bool is_direction = KvMetaDataBuffer::instance()->isDirectionInDegreesParam(st.sensor.paramId);
  bool direction_start_0 = false, direction_stop_0 = false;

  mSelectedObs.clear();
  for (int r=minRow; r<=maxRow; ++r) {
    const SensorTime st = tableModel->findSensorTime(tableModel->index(r, minCol));
    if (r == minRow or r == maxRow) {
      EditDataPtr obs = mDA->findE(st);
      if (not obs)
        return false;
      if (Helpers::is_missing(obs) or Helpers::is_rejected(obs))
        return false;
      if (is_direction and r == minRow)
        direction_start_0 = obs->corrected() == 0;
      if (is_direction and r == maxRow)
        direction_stop_0 = obs->corrected() == 0;
    }
    if (r == minRow)
      mSelectedStart = st;
    else if (r == maxRow)
      mSelectedEnd = st;
    else
      mSelectedObs.push_back(st);
  }
  if (is_direction and direction_start_0 != direction_stop_0)
    return false;
  return true;
}
