
#include "AutoDataList.hh"

#include "BusyIndicator.h"
#include "ChangeReplay.hh"
#include "ColumnFactory.hh"
#include "DataListAddColumn.hh"
#include "DataListModel.hh"
#include "ViewChanges.hh"

#include <QtGui/QHeaderView>
#include <QtGui/QMenu>
#include <QtGui/QPushButton>

#include <QtXml/QDomElement>

#include <boost/foreach.hpp>

#include "ui_datalist.h"

#define M_TIME
#define MILOGGER_CATEGORY "kvhqc.AutoDataList"
#include "HqcLogging.hh"

namespace {
const std::string VIEW_TYPE = "DataList";
const std::string ID = "1";
}

struct AutoDataList::eq_Column : public std::binary_function<Column, Column, bool> {
  bool operator()(const Column& a, const Column& b) const
    {
      return eq_Sensor()(a.sensor, b.sensor)
          and (a.type == b.type)
          and (a.timeOffset == b.timeOffset);
    }
};

struct AutoDataList::lt_Column : public std::binary_function<Column, Column, bool> {
  bool operator()(const Column& a, const Column& b) const
    {
      if (not eq_Sensor()(a.sensor, b.sensor))
        return lt_Sensor()(a.sensor, b.sensor);
      if (a.type != b.type)
        return a.type < b.type;
      return a.timeOffset < b.timeOffset;
    }
};


AutoDataList::AutoDataList(QWidget* parent)
  : DataList(parent)
{
  mColumnMenu = new QMenu(this);
  mColumnAdd = mColumnMenu->addAction(QIcon("icons:dl_columns_add.svg"), tr("Add column..."), this, SLOT(onActionAddColumn()));
  mColumnRemove = mColumnMenu->addAction(QIcon("icons:dl_columns_remove.svg"), tr("Remove column"), this, SLOT(onActionRemoveColumn()));
  mColumnReset = mColumnMenu->addAction(QIcon("icons:dl_columns_reset.svg"), tr("Reset columns"), this, SLOT(onActionResetColumns()));
  mColumnAdd->setEnabled(false);
  mColumnRemove->setEnabled(false);
  mColumnReset->setEnabled(false);

  QPushButton* buttonColumns = new QPushButton(tr("Columns"), this);
  buttonColumns->setIcon(QIcon("icons:dl_columns.svg"));
  buttonColumns->setMenu(mColumnMenu);
  ui->layoutButtons->addWidget(buttonColumns);

  // mButtonJump = new QPushButton(tr("Follow"), this);
  // //buttonJump->setIcon(QIcon("icons:dl_columns.svg"));
  // //buttonJump->setMenu(mColumnMenu);
  // mButtonJump->setCheckable(true);
  // mButtonJump->setChecked(false);
  // ui->layoutButtons->addWidget(mButtonJump);

  QPushButton* buttonEarlier = new QPushButton("+", this);
  buttonEarlier->setToolTip(tr("Earlier"));
  connect(buttonEarlier, SIGNAL(clicked()), this, SLOT(onEarlier()));
  ui->table->addScrollBarWidget(buttonEarlier, Qt::AlignTop);

  QPushButton* buttonLater = new QPushButton("+", this);
  buttonLater->setToolTip(tr("Later"));
  connect(buttonLater, SIGNAL(clicked()), this, SLOT(onLater()));
  ui->table->addScrollBarWidget(buttonLater, Qt::AlignBottom);

  ui->table->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->table->horizontalHeader(), SIGNAL(customContextMenuRequested(const QPoint&)),
      this, SLOT(onHorizontalHeaderContextMenu(const QPoint&)));
  connect(ui->table->horizontalHeader(), SIGNAL(sectionMoved(int, int, int)),
      this, SLOT(onHorizontalHeaderSectionMoved(int, int, int)));
}

AutoDataList::~AutoDataList()
{
  storeChanges();
}

void AutoDataList::navigateTo(const SensorTime& st)
{
  METLIBS_LOG_TIME();
  if (not st.valid() or eq_SensorTime()(mSensorTime, st))
    return;

  if (not mEmittingNavigateTo or not mTableModel.get() or mTableModel->findIndexes(st).empty()) {
    mTimeLimits = ViewChanges::defaultTimeLimits(st);
    mOriginalTimeLimits = mTimeLimits;
    if (not mSensorTime.valid() or not eq_Sensor()(mSensorTime.sensor, st.sensor)) {
      // need to update related parameters and neighbor list
      storeChanges();
      
      // set original columns
      mSensorTime = st;
      generateColumns();
      
      replay(ViewChanges::fetch(mSensorTime.sensor, VIEW_TYPE, ID));
    }
    makeModel();
  }
  DataList::navigateTo(st);
}

void AutoDataList::generateColumns()
{
  METLIBS_LOG_SCOPE();
  mColumns = Columns_t();
  
  const std::vector<Sensor> sensors = Helpers::relatedSensors(mSensorTime, VIEW_TYPE);
  BOOST_FOREACH(const Sensor& s, sensors) {
    Column c;
    c.sensor = s;
    c.timeOffset = 0;
    if (s.stationId == mSensorTime.sensor.stationId and s.paramId == mSensorTime.sensor.paramId) {
      c.type = ORIGINAL;
      mColumns.push_back(c);
    }
    c.type = CORRECTED;
    mColumns.push_back(c);
    METLIBS_LOG_DEBUG("gen " << s);
  }
  mOriginalColumns = mColumns;
}

void AutoDataList::makeModel()
{
  METLIBS_LOG_SCOPE();
  BusyIndicator busy;
  std::auto_ptr<DataListModel> newModel(new DataListModel(mDA, mTimeLimits));
  BOOST_FOREACH(const Column& c, mColumns) {
    ObsColumnPtr oc = makeColumn(c);
    if (oc)
      newModel->addColumn(oc);
  }
  newModel->setCenter(mSensorTime.sensor.stationId);
  updateModel(newModel.release());
  mColumnAdd->setEnabled(true);
}

ObsColumnPtr AutoDataList::makeColumn(const Column& c)
{
  boost::posix_time::time_duration toff = boost::posix_time::hours(c.timeOffset);

  if (c.type == MODEL) {
    ModelColumnPtr mc = ColumnFactory::columnForSensor(mMA, c.sensor, mTimeLimits);
    mc->setTimeOffset(toff);
    return mc;
  } else {
    ObsColumn::Type cdt = ObsColumn::NEW_CORRECTED;
    if (c.type == ORIGINAL)
      cdt = ObsColumn::ORIGINAL;
    else if (c.type == FLAGS)
      cdt = ObsColumn::NEW_CONTROLINFO;
    DataColumnPtr dc = ColumnFactory::columnForSensor(mDA, c.sensor, mTimeLimits, cdt);
    if (dc)
      dc->setTimeOffset(toff);
    return dc;
  }
}

void AutoDataList::storeChanges()
{
  METLIBS_LOG_SCOPE();
  if (mSensorTime.valid())
    ViewChanges::store(mSensorTime.sensor, VIEW_TYPE, ID, changes());
}

namespace /* anonymous */ {
static const char C_ATTR_STATIONID[] = "stationid";
static const char C_ATTR_PARAMID[]   = "paramid";
static const char C_ATTR_TYPEID[]    = "typeid";
static const char C_ATTR_SENSORNR[]  = "sensornr";
static const char C_ATTR_LEVEL[]     = "level";
static const char C_ATTR_CTYPE[]     = "ctype";
static const char C_ATTR_TOFFSET[]   = "timeoffset";

static const char T_ATTR_START[] = "start";
static const char T_ATTR_END[]   = "end";

static const char E_TAG_CHANGES[] = "changes";
static const char E_TAG_COLUMNS[] = "columns";
static const char E_TAG_REMOVED[] = "removed";
static const char E_TAG_COLUMN[]  = "column";
static const char E_TAG_TSHIFT[]  = "timeshift";
} // anonymous namespace

void AutoDataList::Column::toText(QDomElement& ce) const
{
  ce.setAttribute(C_ATTR_STATIONID, sensor.stationId);
  ce.setAttribute(C_ATTR_PARAMID,   sensor.paramId);
  ce.setAttribute(C_ATTR_TYPEID,    sensor.typeId);
  if (sensor.level != 0)
    ce.setAttribute(C_ATTR_LEVEL, sensor.level);
  if (sensor.sensor != 0)
    ce.setAttribute(C_ATTR_SENSORNR, sensor.sensor);
  QString ctype;
  switch (type) {
  case CORRECTED: ctype = "CORRECTED"; break;
  case ORIGINAL:  ctype = "ORIGINAL";  break;
  case FLAGS:     ctype = "FLAGS";     break;
  case MODEL:     ctype = "MODEL";     break;
  }
  ce.setAttribute(C_ATTR_CTYPE, ctype);
  if (timeOffset != 0)
    ce.setAttribute(C_ATTR_TOFFSET, timeOffset);
}

void AutoDataList::Column::fromText(const QDomElement& ce)
{
  sensor.stationId = ce.attribute(C_ATTR_STATIONID).toInt();
  sensor.paramId   = ce.attribute(C_ATTR_PARAMID)  .toInt();
  sensor.typeId    = ce.attribute(C_ATTR_TYPEID)   .toInt();
  if (ce.hasAttribute(C_ATTR_LEVEL))
    sensor.level = ce.attribute(C_ATTR_LEVEL).toInt();
  if (ce.hasAttribute(C_ATTR_SENSORNR))
    sensor.sensor = ce.attribute(C_ATTR_SENSORNR).toInt();
  const QString ctype = ce.attribute(C_ATTR_CTYPE);
  if      (ctype == "CORRECTED") type = CORRECTED;
  else if (ctype == "ORIGINAL")  type = ORIGINAL;
  else if (ctype == "FLAGS")     type = FLAGS;
  else if (ctype == "MODEL")     type = MODEL;
  if (ce.hasAttribute(C_ATTR_TOFFSET))
    timeOffset = ce.attribute(C_ATTR_TOFFSET).toInt();
  else
    timeOffset = 0;
}

std::string AutoDataList::changes()
{
  METLIBS_LOG_SCOPE();
  QDomDocument doc("changes");
  QDomElement doc_changes = doc.createElement(E_TAG_CHANGES);
  doc.appendChild(doc_changes);

  ChangeReplay<Column, lt_Column> cr;
  const Columns_t removed = cr.removals(mOriginalColumns, mColumns);
  if (not removed.empty()) {
    QDomElement doc_removed = doc.createElement(E_TAG_REMOVED);
    BOOST_FOREACH(const Column& r, removed) {
      QDomElement doc_column = doc.createElement(E_TAG_COLUMN);
      r.toText(doc_column);
      doc_removed.appendChild(doc_column);
    } 
    doc_changes.appendChild(doc_removed);
  }

  QDomElement doc_columns = doc.createElement(E_TAG_COLUMNS);
  BOOST_FOREACH(const Column& c, mColumns) {
    QDomElement doc_column = doc.createElement(E_TAG_COLUMN);
    c.toText(doc_column);
    doc_columns.appendChild(doc_column);
  } 
  doc_changes.appendChild(doc_columns);

  if (mOriginalTimeLimits.t0() != mTimeLimits.t0() or mOriginalTimeLimits.t1() != mTimeLimits.t1()) {
    QDomElement doc_timeshift = doc.createElement(E_TAG_TSHIFT);
    doc_timeshift.setAttribute(T_ATTR_START, (mTimeLimits.t0() - mOriginalTimeLimits.t0()).hours());
    doc_timeshift.setAttribute(T_ATTR_END,   (mTimeLimits.t1() - mOriginalTimeLimits.t1()).hours());
    doc_changes.appendChild(doc_timeshift);
  }

  METLIBS_LOG_DEBUG("changes for " << mSensorTime << ": " << doc.toString());
  return doc.toString().toStdString();
}

void AutoDataList::replay(const std::string& changesText)
{
  METLIBS_LOG_SCOPE();
  METLIBS_LOG_DEBUG("replaying '" << changesText << "'");

  if (changesText.empty())
    return;

  QDomDocument doc;
  doc.setContent(QString::fromStdString(changesText));

  const QDomElement doc_changes = doc.documentElement();

  Columns_t removed;
  const QDomElement doc_removed = doc_changes.firstChildElement(E_TAG_REMOVED);
  if (not doc_removed.isNull()) {
    for(QDomElement c = doc_removed.firstChildElement(E_TAG_COLUMN); not c.isNull(); c = c.nextSiblingElement(E_TAG_COLUMN)) {
      Column col;
      col.fromText(c);
      removed.push_back(col);
      METLIBS_LOG_DEBUG("removed " << col.sensor);
    }
  }

  Columns_t actual;
  const QDomElement doc_actual = doc_changes.firstChildElement(E_TAG_COLUMNS);
  if (not doc_actual.isNull()) {
    for(QDomElement c = doc_actual.firstChildElement(E_TAG_COLUMN); not c.isNull(); c = c.nextSiblingElement(E_TAG_COLUMN)) {
      Column col;
      col.fromText(c);
      actual.push_back(col);
      METLIBS_LOG_DEBUG("column " << col.sensor);
    }
  }
  
  ChangeReplay<Column, lt_Column> cr;
  mColumns = cr.replay(mOriginalColumns, actual, removed);
  
  TimeRange newTimeLimits(mOriginalTimeLimits);
  const QDomElement doc_timeshift = doc_changes.firstChildElement(E_TAG_TSHIFT);
  if (not doc_timeshift.isNull()) {
    const int dT0 = doc_timeshift.attribute(T_ATTR_START).toInt();
    const int dT1 = doc_timeshift.attribute(T_ATTR_END)  .toInt();
    const timeutil::ptime t0 = mOriginalTimeLimits.t0() + boost::posix_time::hours(dT0);
    const timeutil::ptime t1 = mOriginalTimeLimits.t1() + boost::posix_time::hours(dT1);
    TimeRange newTimeLimits(t0, t1);
    METLIBS_LOG_DEBUG(LOGVAL(newTimeLimits));
  }
  mTimeLimits = newTimeLimits;

  bool changed = (mColumns.size() != mOriginalColumns.size())
      or mTimeLimits != mOriginalTimeLimits;
  if (not changed)
    changed = not std::equal(mColumns.begin(), mColumns.end(), mOriginalColumns.begin(), eq_Column());
  mColumnReset->setEnabled(changed);
}

void AutoDataList::onEarlier()
{
  mTimeLimits = TimeRange(mTimeLimits.t0() - boost::posix_time::hours(24), mTimeLimits.t1());
  makeModel();
}

void AutoDataList::onLater()
{
  mTimeLimits = TimeRange(mTimeLimits.t0(), mTimeLimits.t1() + boost::posix_time::hours(24));
  makeModel();
}

void AutoDataList::onHorizontalHeaderContextMenu(const QPoint& pos)
{
  QMenu menu;
  QAction* actionAdd = menu.addAction(mColumnAdd->icon(), mColumnAdd->text());
  QAction* actionDel = menu.addAction(mColumnRemove->icon(), mColumnRemove->text());
  
  QAction* chosen = menu.exec(mapToGlobal(pos));
  if (chosen == 0)
    return;
  
  const int column = ui->table->horizontalHeader()->logicalIndexAt(pos);
  if (chosen == actionAdd) {
    addColumnBefore(column);
  } else if (chosen == actionDel) {
    removeColumns(std::vector<int>(1, column));
  }
}

void AutoDataList::onActionAddColumn()
{
  const QItemSelectionModel* sm = ui->table->selectionModel();
  if (not sm)
    return;

  int column = mTableModel->columnCount(QModelIndex());
  if (const QItemSelectionModel* sm = ui->table->selectionModel()) {
    const QModelIndexList sc = sm->selectedColumns();
    if (sc.size() == 1)
      column = sc.front().column();
  }
  addColumnBefore(column);
}

void AutoDataList::addColumnBefore(int column)
{
  const int columnCount = mTableModel->columnCount(QModelIndex());
  if (column < 0 or column > columnCount)
    column = columnCount;

  DataListAddColumn dac(this);
  if (dac.exec() != QDialog::Accepted)
    return;

  BusyIndicator busy;
  Column c;
  c.sensor = dac.selectedSensor();
  c.type = dac.selectedColumnType();
  c.timeOffset = dac.selectedTimeOffset();
  mColumns.insert(mColumns.begin() + column, c);
  mTableModel->insertColumn(column, makeColumn(c));
  mColumnReset->setEnabled(true);
}

void AutoDataList::removeColumns(std::vector<int> columns)
{
  if (columns.empty())
    return;
  std::sort(columns.begin(), columns.end(), std::greater<int>());
  BOOST_FOREACH(int c, columns) {
    if (c >= 0 and c < (int)mColumns.size())
      mColumns.erase(mColumns.begin() + c);
  }
  makeModel();
  mColumnReset->setEnabled(true);
}

void AutoDataList::onActionRemoveColumn()
{
  const QItemSelectionModel* sm = ui->table->selectionModel();
  if (not sm)
    return;

  std::vector<int> columns;
  const QModelIndexList sc = sm->selectedColumns();
  Q_FOREACH(const QModelIndex& idx, sc)
      columns.push_back(idx.column());
  removeColumns(columns);
}

void AutoDataList::onActionResetColumns()
{
  mTimeLimits = mOriginalTimeLimits;
  mColumns = mOriginalColumns;
  makeModel();
  mColumnReset->setEnabled(false);
}

void AutoDataList::onSelectionChanged(const QItemSelection&, const QItemSelection&)
{
  bool enabled = false;
  if (const QItemSelectionModel* sm = ui->table->selectionModel())
    enabled = not sm->selectedColumns().isEmpty();
  mColumnRemove->setEnabled(enabled);
}

void AutoDataList::onHorizontalHeaderSectionMoved(int logicalIndex, int oVis, int nVis)
{
  if (logicalIndex != oVis)
    return;

  const int from = logicalIndex, to = nVis;
  ui->table->horizontalHeader()->moveSection(to, from);
  mTableModel->moveColumn(from, to); // move back and switch model columns instead

  const Column c = mColumns[from];
  mColumns.erase(mColumns.begin() + from);
  mColumns.insert(mColumns.begin() + to, c);

  mColumnReset->setEnabled(true);

  //ui->table->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}
