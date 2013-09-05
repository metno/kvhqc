
#include "DataList.hh"

#include "BusyIndicator.h"
#include "ChangeReplay.hh"
#include "ColumnFactory.hh"
#include "DataListAddColumn.hh"
#include "DataListModel.hh"
#include "ObsDelegate.hh"

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QFileDialog>
#include <QtGui/QFont>
#include <QtGui/QHeaderView>
#include <QtGui/QMenu>
#include <QtGui/QPushButton>

#include <QtXml/QDomElement>

#include <boost/foreach.hpp>

#include <set>

#include "ui_datalist.h"
#include "ui_dl_addcolumn.h"

#define MILOGGER_CATEGORY "kvhqc.DataList"
#include "HqcLogging.hh"

struct DataList::lt_Column : public std::binary_function<Column, Column, bool> {
  bool operator()(const Column& a, const Column& b) const
    {
      if (not eq_Sensor()(a.sensor, b.sensor))
        return lt_Sensor()(a.sensor, b.sensor);
      if (a.type != b.type)
        return a.type < b.type;
      return a.timeOffset < b.timeOffset;
    }
};

DataList::DataList(QWidget* parent)
  : QWidget(parent)
  , ui(new Ui::DataList)
  , mShowDistances(false)
{
  ui->setupUi(this);
  ui->buttonSaveAs->setIcon(QIcon("icons:dl_save_as.svg"));

  mColumnMenu = new QMenu(this);
  mColumnAdd = mColumnMenu->addAction(QIcon("icons:dl_columns_add.svg"), tr("Add column..."), this, SLOT(onActionAddColumn()));
  mColumnRemove = mColumnMenu->addAction(QIcon("icons:dl_columns_remove.svg"), tr("Remove column"), this, SLOT(onActionRemoveColumn()));
  mColumnReset = mColumnMenu->addAction(QIcon("icons:dl_columns_reset.svg"), tr("Reset columns"), this, SLOT(onActionResetColumns()));
  mColumnAdd->setEnabled(false);
  mColumnRemove->setEnabled(false);
  mColumnReset->setEnabled(false);

  ui->buttonColumnMenu->setIcon(QIcon("icons:dl_columns.svg"));
  ui->buttonColumnMenu->setMenu(mColumnMenu);

  QPushButton* buttonEarlier = new QPushButton("+", this);
  buttonEarlier->setToolTip(tr("Earlier"));
  connect(buttonEarlier, SIGNAL(clicked()), this, SLOT(onEarlier()));
  ui->table->addScrollBarWidget(buttonEarlier, Qt::AlignTop);

  QPushButton* buttonLater = new QPushButton("+", this);
  buttonLater->setToolTip(tr("Later"));
  connect(buttonLater, SIGNAL(clicked()), this, SLOT(onLater()));
  ui->table->addScrollBarWidget(buttonLater, Qt::AlignBottom);
  ui->table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  ui->table->setSelectionBehavior(QAbstractItemView::SelectItems);
  ui->table->setSelectionMode(QAbstractItemView::ExtendedSelection);
  ui->table->setItemDelegate(new ObsDelegate(this));

  QFont mono("Monospace");
  //ui->table->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
  ui->table->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  ui->table->horizontalHeader()->setMovable(true);
  ui->table->verticalHeader()->setFont(mono);
  ui->table->verticalHeader()->setDefaultSectionSize(20);

  ui->table->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->table->horizontalHeader(), SIGNAL(customContextMenuRequested(const QPoint&)),
      this, SLOT(onHorizontalHeaderContextMenu(const QPoint&)));
  connect(ui->table->horizontalHeader(), SIGNAL(sectionMoved(int, int, int)),
      this, SLOT(onHorizontalHeaderSectionMoved(int, int, int)));
  connect(ui->table, SIGNAL(currentChanged(const QModelIndex&)),
      this, SLOT(currentChanged(const QModelIndex&)));
}

DataList::~DataList()
{
}

void DataList::setSensorsAndTimes(const Sensors_t& sensors, const TimeRange& limits)
{
  DataView::setSensorsAndTimes(sensors, limits);
  mOriginalTimeLimits = limits;
  mTimeLimits = limits;

  mColumns = Columns_t();
  BOOST_FOREACH(const Sensor& s, sensors) {
    Column c;
    c.sensor = s;
    c.type = ORIGINAL;
    c.timeOffset = 0;
    mColumns.push_back(c);
    c.type = CORRECTED;
    mColumns.push_back(c);
  }
  mOriginalColumns = mColumns;

  updateModel();
}

void DataList::updateModel()
{
  BusyIndicator busy;
  std::auto_ptr<DataListModel> newModel(new DataListModel(mDA, mTimeLimits));
  BOOST_FOREACH(const Column& c, mColumns) {
    ObsColumnPtr oc = makeColumn(c);
    if (oc)
      newModel->addColumn(oc);
  }
  if (mShowDistances and not mColumns.empty())
    newModel->setCenter(mColumns[0].sensor.stationId);
  else
    newModel->setCenter(0);
  
  mTableModel = newModel;
  ui->table->setModel(mTableModel.get());
  connect(ui->table->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
      this, SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));
  
  ui->buttonsAcceptReject->updateModel(mDA, ui->table);
  mColumnAdd->setEnabled(true);
}

ObsColumnPtr DataList::makeColumn(const Column& c)
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

void DataList::navigateTo(const SensorTime& st)
{
  METLIBS_LOG_SCOPE();
  if (not st.valid() or eq_SensorTime()(mSensorTime, st))
    return;

  mSensorTime = st;
  METLIBS_LOG_DEBUG(LOGVAL(mSensorTime));

  const QModelIndexList idxs = mTableModel->findIndexes(st);

  const QModelIndex& currentIdx = ui->table->currentIndex();
  QItemSelection selection;
  bool scroll = (not idxs.empty());
  BOOST_FOREACH(const QModelIndex idx, idxs) {
    selection.select(idx, idx);
    if (idx == currentIdx)
      scroll = false;
  }
  if (scroll) {
    ui->table->scrollTo(idxs.front(), QAbstractItemView::PositionAtCenter);
    ui->table->scrollTo(idxs.back(), QAbstractItemView::PositionAtCenter);
  }
  if (QItemSelectionModel* sm = ui->table->selectionModel())
    sm->select(selection, QItemSelectionModel::ClearAndSelect);
}

void DataList::currentChanged(const QModelIndex& current)
{
  METLIBS_LOG_SCOPE();
  const SensorTime st = mTableModel->findSensorTime(current);
  METLIBS_LOG_DEBUG(LOGVAL(st));
  if (st.valid() and not eq_SensorTime()(mSensorTime, st)) {
    mSensorTime = st;
    /*emit*/ signalNavigateTo(st);
  }
}

void DataList::onEarlier()
{
  mTimeLimits = TimeRange(mTimeLimits.t0() - boost::posix_time::hours(24), mTimeLimits.t1());
  updateModel();
}

void DataList::onLater()
{
  mTimeLimits = TimeRange(mTimeLimits.t0(), mTimeLimits.t1() + boost::posix_time::hours(24));
  updateModel();
}

void DataList::onHorizontalHeaderContextMenu(const QPoint& pos)
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

void DataList::onActionAddColumn()
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

void DataList::addColumnBefore(int column)
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

void DataList::removeColumns(std::vector<int> columns)
{
  if (columns.empty())
    return;
  std::sort(columns.begin(), columns.end(), std::greater<int>());
  BOOST_FOREACH(int c, columns) {
    if (c >= 0 and c < (int)mColumns.size())
      mColumns.erase(mColumns.begin() + c);
  }
  updateModel();
  mColumnReset->setEnabled(true);
}

void DataList::onActionRemoveColumn()
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

void DataList::onActionResetColumns()
{
  mTimeLimits = mOriginalTimeLimits;
  mColumns = mOriginalColumns;
  updateModel();
  mColumnReset->setEnabled(false);
}

void DataList::onSelectionChanged(const QItemSelection&, const QItemSelection&)
{
  bool enabled = false;
  if (const QItemSelectionModel* sm = ui->table->selectionModel())
    enabled = not sm->selectedColumns().isEmpty();
  mColumnRemove->setEnabled(enabled);
}

namespace /* anonymous */ {
static const char C_ATTR_STATIONID[] = "stationid";
static const char C_ATTR_PARAMID[]   = "paramid";
static const char C_ATTR_TYPEID[]    = "typeid";
static const char C_ATTR_CTYPE[]     = "ctype";
static const char C_ATTR_TOSSFET[]   = "timeoffset";

static const char T_ATTR_START[] = "start";
static const char T_ATTR_END[]   = "end";

static const char E_TAG_CHANGES[] = "changes";
static const char E_TAG_COLUMNS[] = "columns";
static const char E_TAG_REMOVED[] = "removed";
static const char E_TAG_COLUMN[]  = "column";
static const char E_TAG_TSHIFT[]  = "timeshift";
} // anonymous namespace

void DataList::Column::toText(QDomElement& ce) const
{
  ce.setAttribute(C_ATTR_STATIONID, sensor.stationId);
  ce.setAttribute(C_ATTR_PARAMID,   sensor.paramId);
  ce.setAttribute(C_ATTR_TYPEID,    sensor.typeId);
  QString ctype;
  switch (type) {
  case CORRECTED: ctype = "CORRECTED"; break;
  case ORIGINAL:  ctype = "ORIGINAL";  break;
  case FLAGS:     ctype = "FLAGS";     break;
  case MODEL:     ctype = "MODEL";     break;
  }
  ce.setAttribute(C_ATTR_CTYPE, ctype);
  if (timeOffset != 0)
    ce.setAttribute(C_ATTR_TOSSFET, timeOffset);
}

void DataList::Column::fromText(const QDomElement& ce)
{
  sensor.stationId = ce.attribute(C_ATTR_STATIONID).toInt();
  sensor.paramId   = ce.attribute(C_ATTR_PARAMID)  .toInt();
  sensor.typeId    = ce.attribute(C_ATTR_TYPEID)   .toInt();
  const QString ctype = ce.attribute(C_ATTR_CTYPE);
  if      (ctype == "CORRECTED") type = CORRECTED;
  else if (ctype == "ORIGINAL")  type = ORIGINAL;
  else if (ctype == "FLAGS")     type = FLAGS;
  else if (ctype == "MODEL")     type = MODEL;
  if (ce.hasAttribute(C_ATTR_TOSSFET))
    timeOffset = ce.attribute(C_ATTR_TOSSFET).toInt();
  else
    timeOffset = 0;
}

void DataList::onHorizontalHeaderSectionMoved(int logicalIndex, int oVis, int nVis)
{
  if (logicalIndex != oVis)
    return;

  const int from = logicalIndex, to = nVis;
  ui->table->horizontalHeader()->moveSection(to, from);
  mTableModel->moveColumn(from, to); // move back and switch model columns instead

  const Column c = mColumns[from];
  mColumns.erase(mColumns.begin() + from);
  mColumns.insert(mColumns.begin() + to, c);

  mColumnReset->setEnabled(false);

  //ui->table->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

std::string DataList::changes()
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

  METLIBS_LOG_DEBUG("changes=" << doc.toString());
  return doc.toString().toStdString();
}

void DataList::replay(const std::string& changesText)
{
  METLIBS_LOG_SCOPE();
  METLIBS_LOG_DEBUG("replaying " << changesText);

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
      METLIBS_LOG_DEBUG("removed " << col.sensor.stationId << ';' << col.sensor.paramId << ';' << col.sensor.typeId);
    }
  }

  Columns_t actual;
  const QDomElement doc_actual = doc_changes.firstChildElement(E_TAG_COLUMNS);
  if (not doc_actual.isNull()) {
    for(QDomElement c = doc_actual.firstChildElement(E_TAG_COLUMN); not c.isNull(); c = c.nextSiblingElement(E_TAG_COLUMN)) {
      Column col;
      col.fromText(c);
      actual.push_back(col);
      METLIBS_LOG_DEBUG("column " << col.sensor.stationId << ';' << col.sensor.paramId << ';' << col.sensor.typeId);
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

  updateModel();
}

std::string DataList::type() const
{
  return "DataList";
}

std::string DataList::id() const
{
  return "1";
}

namespace /* anonymous */ {
QString protectForCSV(QString txt)
{
  txt.replace('\n', " ");
  txt.replace('\"', "'");
  txt.replace('\t', " ");
  return txt;
}
QString protectForCSV(const QVariant& v)
{
  return protectForCSV(v.toString());
}
} // namespace anonymous

void DataList::onButtonSaveAs()
{
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save Table"),
      "datalist.csv",
      tr("CSV Table (*.csv)"));
  if (fileName.isEmpty())
    return;
  
  QFile file(fileName);
  file.open(QIODevice::WriteOnly | QIODevice::Text);

  int r0, r1, c0, c1;

  std::set<int> selectedRows, selectedColumns;
  std::set< std::pair<int,int> > selectedCells;
  QModelIndexList selected;
  if (QItemSelectionModel* sm = ui->table->selectionModel())
    selected = sm->selectedIndexes();
  if (not selected.isEmpty()) {
    for (int i=0; i<selected.count(); i++) {
      const int r = selected.at(i).row(), c = selected.at(i).column();
      selectedRows.insert(r);
      selectedColumns.insert(c);
      selectedCells.insert(std::make_pair(r, c));
    }
    r0 = *selectedRows.begin();
    r1 = *selectedRows.rbegin();
    c0 = *selectedColumns.begin();
    c1 = *selectedColumns.rbegin();
  } else {
    r0 = 0;
    r1 = mTableModel->rowCount(QModelIndex())-1; // no problem if < 0
    c0 = 0;
    c1 = mTableModel->columnCount(QModelIndex())-1; // no problem if < 0
  }

  QTextStream out(&file);

  out << "\"\"";
  for (int c=c0; c<=c1; ++c) {
    if (selectedColumns.empty() or selectedColumns.find(c) != selectedColumns.end())
      out << "\t\"" << protectForCSV(mTableModel->headerData(c, Qt::Horizontal, Qt::ToolTipRole)) << '\"';
  }
  out << "\n";

  for (int r=r0; r<=r1; ++r) {
    if (selectedRows.empty() or selectedRows.find(r) != selectedRows.end()) {
      out << '\"' << protectForCSV(mTableModel->headerData(r, Qt::Vertical, Qt::DisplayRole)) << '\"';
      for (int c=c0; c<=c1; ++c) {
        if (selectedColumns.empty() or selectedColumns.find(c) != selectedColumns.end()) {
          QString cell;
          if (selectedCells.empty() or selectedCells.find(std::make_pair(r, c)) != selectedCells.end())
            cell = protectForCSV(mTableModel->data(mTableModel->index(r, c), Qt::DisplayRole));
          out << "\t\"" << cell << '\"';
        }
      }
      out << "\n";
    }
  }
 
  file.close(); 
}
