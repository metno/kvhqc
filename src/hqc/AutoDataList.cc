
#include "AutoDataList.hh"

#include "DataListAddColumn.hh"

#include "common/ColumnFactory.hh"
#include "common/DataListModel.hh"
#include "common/KvHelpers.hh"

#include "util/ChangeReplay.hh"
#include "util/BusyIndicator.hh"

#include <QtGui/QHeaderView>
#include <QtGui/QMenu>
#include <QtGui/QPushButton>

#include <QtXml/QDomElement>

#include <boost/foreach.hpp>

#include "ui_datalist.h"

#define MILOGGER_CATEGORY "kvhqc.AutoDataList"
#include "common/ObsLogging.hh"

namespace /* anonymous */ {
static const char C_ATTR_STATIONID[] = "stationid";
static const char C_ATTR_PARAMID[]   = "paramid";
static const char C_ATTR_TYPEID[]    = "typeid";
static const char C_ATTR_SENSORNR[]  = "sensornr";
static const char C_ATTR_LEVEL[]     = "level";
static const char C_ATTR_CTYPE[]     = "ctype";
static const char C_ATTR_TOFFSET[]   = "timeoffset";

static const char E_TAG_COLUMNS[] = "columns";
static const char E_TAG_REMOVED[] = "removed";
static const char E_TAG_COLUMN[]  = "column";

const std::string VIEW_TYPE = "DataList";
} // anonymous namespace

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
  : TimespanDataList(parent)
{
  mColumnMenu = new QMenu(this);
  mColumnAdd = mColumnMenu->addAction(QIcon("icons:dl_columns_add.svg"), tr("Add column..."), this, SLOT(onActionAddColumn()));
  mColumnRemove = mColumnMenu->addAction(QIcon("icons:dl_columns_remove.svg"), tr("Remove column"), this, SLOT(onActionRemoveColumn()));
  mColumnReset = mColumnMenu->addAction(QIcon("icons:dl_columns_reset.svg"), tr("Reset columns"), this, SLOT(onActionResetColumns()));
  mColumnAdd->setEnabled(false);
  mColumnRemove->setEnabled(false);
  mColumnReset->setEnabled(false);

  mButtonColumns = new QPushButton(tr("Columns"), this);
  mButtonColumns->setIcon(QIcon("icons:dl_columns.svg"));
  mButtonColumns->setMenu(mColumnMenu);
  ui->layoutButtons->addWidget(mButtonColumns);

  ui->table->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->table->horizontalHeader(), SIGNAL(customContextMenuRequested(const QPoint&)),
      this, SLOT(onHorizontalHeaderContextMenu(const QPoint&)));
  connect(ui->table->horizontalHeader(), SIGNAL(sectionMoved(int, int, int)),
      this, SLOT(onHorizontalHeaderSectionMoved(int, int, int)));
}

AutoDataList::~AutoDataList()
{
  // cannot be called from ~DynamicDataView as it calls virtual methods
  storeChanges();
}

void AutoDataList::retranslateUi()
{
  mColumnAdd->setText(tr("Add column..."));
  mColumnRemove->setText(tr("Remove column"));
  mColumnReset->setText(tr("Reset columns"));
  mButtonColumns->setText(tr("Columns"));

  TimespanDataList::retranslateUi();
}

void AutoDataList::switchSensorPrepare()
{
  METLIBS_LOG_SCOPE();
  TimespanDataList::switchSensorPrepare();

  mColumns = Columns_t();
  
  const std::vector<Sensor> sensors = Helpers::relatedSensors(currentSensorTime().sensor, timeSpan(), VIEW_TYPE);
  BOOST_FOREACH(const Sensor& s, sensors) {
    Column c;
    c.sensor = s;
    c.timeOffset = 0;
    if (s.stationId == currentSensorTime().sensor.stationId and s.paramId == currentSensorTime().sensor.paramId) {
      c.type = ORIGINAL;
      mColumns.push_back(c);
    }
    c.type = CORRECTED;
    mColumns.push_back(c);
    METLIBS_LOG_DEBUG("gen " << s);
  }
  mOriginalColumns = mColumns;
}

void AutoDataList::loadChangesXML(const QDomElement& doc_changes)
{
  TimespanDataList::loadChangesXML(doc_changes);

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
  
  mColumnReset->setEnabled(hasChangedColumns());
}

void AutoDataList::storeChangesXML(QDomElement& doc_changes)
{
  TimespanDataList::storeChangesXML(doc_changes);

  if (not hasChangedColumns())
    return;

  QDomDocument doc = doc_changes.ownerDocument();

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
}

bool AutoDataList::hasChangedColumns() const
{
  return (mColumns.size() != mOriginalColumns.size())
      or not std::equal(mColumns.begin(), mColumns.end(), mOriginalColumns.begin(), eq_Column());
}

DataListModel* AutoDataList::makeModel()
{
  METLIBS_LOG_SCOPE();
  //BusyIndicator busy;
  std::auto_ptr<DataListModel> newModel(new DataListModel(mDA, timeSpan()));
  BOOST_FOREACH(const Column& c, mColumns) {
    ObsColumn_p oc = makeColumn(c);
    if (oc)
      newModel->addColumn(oc);
  }
  newModel->setCenter(currentSensorTime().sensor.stationId);
  mColumnAdd->setEnabled(true);

  return newModel.release();
}

ObsColumn_p AutoDataList::makeColumn(const Column& c)
{
  boost::posix_time::time_duration toff = boost::posix_time::hours(c.timeOffset);

  if (c.type == MODEL) {
    ModelColumn_p mc = ColumnFactory::columnForSensor(mMA, c.sensor, timeSpan());
    if (mc and c.timeOffset != 0)
      mc->setTimeOffset(boost::posix_time::hours(c.timeOffset));
    return mc;
  } else {
    ObsColumn::Type cdt = ObsColumn::NEW_CORRECTED;
    if (c.type == ORIGINAL)
      cdt = ObsColumn::ORIGINAL;
    else if (c.type == FLAGS)
      cdt = ObsColumn::NEW_CONTROLINFO;
    DataColumn_p dc = ColumnFactory::columnForSensor(mDA, c.sensor, timeSpan(), cdt);
    if (dc and c.timeOffset != 0)
      dc->setTimeOffset(boost::posix_time::hours(c.timeOffset));
    return dc;
  }
}

std::string AutoDataList::viewType() const
{
  return VIEW_TYPE;
}

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
  dac.setSensor(currentSensorTime().sensor);
  if (dac.exec() != QDialog::Accepted)
    return;

  //BusyIndicator busy;
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
  updateModel(makeModel());
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
  mColumns = mOriginalColumns;
  updateModel(makeModel());
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
