
#include "DataList.hh"

#include "AcceptReject.hh"
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

struct DataList::eq_Column : public std::unary_function<Column, bool> {
    const Column& a;
    eq_Column(const Column& c) : a(c) { }
    bool operator()(const Column& b) const
    { return eq_Sensor()(a.sensor, b.sensor) and a.type == b.type and a.timeOffset == b.timeOffset;}
};

DataList::DataList(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::DataList)
{
  ui->setupUi(this);

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

#if 1
#include "../src/icon_accept.xpm"
#include "../src/icon_reject.xpm"
    QIcon iconAccept, iconReject;
    iconAccept.addPixmap(QPixmap(icon_accept));
    iconReject.addPixmap(QPixmap(icon_reject));
    ui->buttonAccept   ->setIcon(iconAccept);
    ui->buttonReject   ->setIcon(iconReject);
#endif

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
    std::auto_ptr<DataListModel> newModel(new DataListModel(mDA, mTimeLimits));
    BOOST_FOREACH(const Column& c, mColumns) {
      ObsColumnPtr oc = makeColumn(c);
      if (oc)
        newModel->addColumn(oc);
    }

    mTableModel = newModel;
    ui->table->setModel(mTableModel.get());
    connect(ui->table->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
        this, SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));

    //ui->table->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
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
    ui->table->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
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
  QAction* actionAdd = menu.addAction(tr("Add column..."));
  QAction* actionDel = menu.addAction(tr("Remove column"));
  
  QAction* chosen = menu.exec(mapToGlobal(pos));
  if (chosen == 0)
    return;
  
  int column = ui->table->horizontalHeader()->logicalIndexAt(pos);
  if (chosen == actionAdd) {
    DataListAddColumn dac(this);
    if (dac.exec() != QDialog::Accepted)
      return;
    
    Column c;
    c.sensor = dac.selectedSensor();
    c.type = dac.selectedColumnType();
    c.timeOffset = dac.selectedTimeOffset();
    mColumns.insert(mColumns.begin() + column, c);
    mTableModel->insertColumn(column, makeColumn(c));
    //ui->table->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
  } else if (chosen == actionDel) {
    mColumns.erase(mColumns.begin() + column);
    updateModel();
  }
}

namespace /* anonymous */ {
static const char C_ATTR_STATIONID[] = "stationid";
static const char C_ATTR_PARAMID[]   = "paramid";
static const char C_ATTR_TYPEID[]    = "typeid";
static const char C_ATTR_CTYPE[]     = "ctype";
static const char C_ATTR_TOSSFET[]   = "timeoffset";

static const char T_ATTR_START[] = "start";
static const char T_ATTR_END[]   = "end";

static const char E_TAG_ADDED[]   = "added";
static const char E_TAG_MOVED[]   = "moved";
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

    //ui->table->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

std::string DataList::changes()
{
    METLIBS_LOG_SCOPE();

    Columns_t reordered;
    for(unsigned int visual=0; visual<mColumns.size(); ++visual) {
        const int logical = ui->table->horizontalHeader()->logicalIndex(visual);
        if (logical >= 0 and logical < (int)mColumns.size()) {
            reordered.push_back(mColumns[logical]);
        } else {
            METLIBS_LOG_ERROR("column without logical index");
        }
    }

    QDomDocument doc("changes");
    QDomElement changes = doc.createElement("changes");
    doc.appendChild(changes);
    unsigned int rNoAdd = 0;
    for(unsigned int r=0; r<reordered.size(); ++r) {
        Columns_t::const_iterator oit = std::find_if(mOriginalColumns.begin(), mOriginalColumns.end(), eq_Column(reordered[r]));
        if (oit == mOriginalColumns.end()) {
            // not in list of original columns => added
            QDomElement added = doc.createElement(E_TAG_ADDED);
            added.setAttribute("at", r);
            changes.appendChild(added);
            QDomElement column = doc.createElement(E_TAG_COLUMN);
            reordered[r].toText(column);
            added.appendChild(column);
        } else {
            // found => moved by (new index - old index))
            const int oidx = (oit - mOriginalColumns.begin());
            int movedBy = rNoAdd - oidx;
            if (movedBy < 0) {
                QDomElement moved = doc.createElement(E_TAG_MOVED);
                moved.setAttribute("by", movedBy);
                changes.appendChild(moved);
                QDomElement column = doc.createElement(E_TAG_COLUMN);
                reordered[r].toText(column);
                moved.appendChild(column);
            }
            rNoAdd += 1;
        }
    } 

    QDomElement removed = doc.createElement(E_TAG_REMOVED);
    BOOST_FOREACH(const Column& o, mOriginalColumns) {
        Columns_t::const_iterator nit = std::find_if(mColumns.begin(), mColumns.end(), eq_Column(o));
        if (nit == mColumns.end()) {
            // not in list of present columns => removed
            QDomElement column = doc.createElement(E_TAG_COLUMN);
            o.toText(column);
            removed.appendChild(column);
        }
    }
    if (removed.hasChildNodes())
        changes.appendChild(removed);

    if (mOriginalTimeLimits.t0() != mTimeLimits.t0() or mOriginalTimeLimits.t1() != mTimeLimits.t1()) {
        QDomElement timeshift = doc.createElement(E_TAG_TSHIFT);
        timeshift.setAttribute(T_ATTR_START, (mTimeLimits.t0() - mOriginalTimeLimits.t0()).hours());
        timeshift.setAttribute(T_ATTR_END,   (mTimeLimits.t1() - mOriginalTimeLimits.t1()).hours());
        changes.appendChild(timeshift);
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

    Columns_t newColumns = mOriginalColumns;
#ifndef NDEBUG
    {
        QDomDocument doc("orig_columns");
        BOOST_FOREACH(const Column& n, newColumns) {
            QDomElement c = doc.createElement(E_TAG_COLUMN);
            n.toText(c);
            doc.appendChild(c);
        }
        METLIBS_LOG_DEBUG(doc.toString().toStdString());
    }
#endif

    const QDomElement changes = doc.documentElement();

    const QDomElement removed = changes.firstChildElement(E_TAG_REMOVED);
    if (not removed.isNull()) {
        for(QDomElement c = removed.firstChildElement(E_TAG_COLUMN); not c.isNull(); c = c.nextSiblingElement(E_TAG_COLUMN)) {
            Column col;
            col.fromText(c);
            Columns_t::iterator it = std::find_if(newColumns.begin(), newColumns.end(), eq_Column(col));
            if (it != newColumns.end()) {
                newColumns.erase(it);
                METLIBS_LOG_DEBUG("removed " << col.sensor.stationId << ';' << col.sensor.paramId << ';' << col.sensor.typeId);
            } else {
                METLIBS_LOG_DEBUG("cannot remove " << col.sensor.stationId << ';' << col.sensor.paramId << ';' << col.sensor.typeId);
            }
        }
    }            

    for(QDomElement m = changes.firstChildElement(E_TAG_MOVED); not m.isNull(); m = m.nextSiblingElement(E_TAG_MOVED)) {
        int by = m.attribute("by").toInt();
        Column col;
        const QDomElement c = m.firstChildElement(E_TAG_COLUMN);
        col.fromText(c);

        Columns_t::iterator oit = std::find_if(mOriginalColumns.begin(), mOriginalColumns.end(), eq_Column(col));
        Columns_t::iterator nit = std::find_if(newColumns.begin(),       newColumns.end(),       eq_Column(col));
        if (oit != mOriginalColumns.end() and nit != newColumns.end()) {
            const int oIdx = (oit - mOriginalColumns.begin()), nIdx = oIdx + by;
            newColumns.erase(nit);
            newColumns.insert(newColumns.begin() + nIdx, col);
            METLIBS_LOG_DEBUG("moved " << col.sensor.stationId << ';' << col.sensor.paramId << ';' << col.sensor.typeId);
        } else {
            METLIBS_LOG_DEBUG("cannot move " << col.sensor.stationId << ';' << col.sensor.paramId << ';' << col.sensor.typeId);
        }
    }

    for(QDomElement a = changes.firstChildElement(E_TAG_ADDED); not a.isNull(); a = a.nextSiblingElement(E_TAG_ADDED)) {
        int at = a.attribute("at").toInt();
        Column col;
        const QDomElement c = a.firstChildElement(E_TAG_COLUMN);
        col.fromText(c);
        newColumns.insert(newColumns.begin() + at, col);
        METLIBS_LOG_DEBUG("added " << col.sensor.stationId << ';' << col.sensor.paramId << ';' << col.sensor.typeId);
    }

#ifndef NDEBUG
    {
        QDomDocument doc("new_columns");
        BOOST_FOREACH(const Column& n, newColumns) {
            QDomElement c = doc.createElement(E_TAG_COLUMN);
            n.toText(c);
            doc.appendChild(c);
        }
        METLIBS_LOG_DEBUG(doc.toString().toStdString());
    }
#endif

    TimeRange newTimeLimits(mOriginalTimeLimits);
    const QDomElement timeshift = changes.firstChildElement(E_TAG_TSHIFT);
    if (not timeshift.isNull()) {
        const int dT0 = timeshift.attribute(T_ATTR_START).toInt();
        const int dT1 = timeshift.attribute(T_ATTR_END)  .toInt();
        const timeutil::ptime t0 = mOriginalTimeLimits.t0() + boost::posix_time::hours(dT0);
        const timeutil::ptime t1 = mOriginalTimeLimits.t1() + boost::posix_time::hours(dT1);
        TimeRange newTimeLimits(t0, t1);
        METLIBS_LOG_DEBUG(LOGVAL(newTimeLimits));
    }

    mColumns = newColumns;
    mTimeLimits = newTimeLimits;
    updateModel();
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
   const QModelIndexList selected = ui->table->selectionModel()->selectedIndexes();
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
     r1 = mTableModel->rowCount(QModelIndex())-1;
     c0 = 0;
     c1 = mTableModel->columnCount(QModelIndex())-1;
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

void DataList::onAccept()
{
  METLIBS_LOG_SCOPE();
  if (not mSelectedObs.empty()) {
    const bool qc2ok = ui->checkQC2->isChecked();
    mDA->newVersion();
    BOOST_FOREACH(SensorTime& st, mSelectedObs) {
      if (mSelectedColumnIsOriginal)
        AcceptReject::accept_original(mDA, st, qc2ok);
      else
        AcceptReject::accept_corrected(mDA, st, qc2ok);
    }
  }
  onSelectionChanged(QItemSelection(), QItemSelection());
}

void DataList::onReject()
{
  METLIBS_LOG_SCOPE();
  if (not mSelectedObs.empty()) {
    const bool qc2ok = ui->checkQC2->isChecked();
    mDA->newVersion();
    BOOST_FOREACH(SensorTime& st, mSelectedObs) {
      AcceptReject::reject(mDA, st, qc2ok);
    }
  }
  onSelectionChanged(QItemSelection(), QItemSelection());
}

void DataList::onSelectionChanged(const QItemSelection&, const QItemSelection&)
{
  METLIBS_LOG_SCOPE();

  QModelIndexList selected = ui->table->selectionModel()->selectedIndexes();
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
      DataColumnPtr dc = boost::dynamic_pointer_cast<DataColumn>(mTableModel->getColumn(minCol));
      if (dc and (dc->type() == ObsColumn::ORIGINAL or dc->type() == ObsColumn::NEW_CORRECTED)) {
        int possible = AcceptReject::ALL;
        for (int r=minRow; r<=maxRow; ++r) {
          const SensorTime st = mTableModel->findSensorTime(mTableModel->index(r, minCol));
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
  ui->buttonAccept->setEnabled(enableAccept);
  ui->buttonReject->setEnabled(enableReject);
}
