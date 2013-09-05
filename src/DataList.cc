
#include "DataList.hh"

#include "ColumnFactory.hh"
#include "DataListModel.hh"

#include <QtGui/QFont>
#include <QtGui/QHeaderView>
#include <QtGui/QMenu>
#include <QtGui/QPushButton>
#include <QtXml/QDomElement>

#include <boost/foreach.hpp>

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
    : QTableView(parent)
{
    QPushButton* buttonEarlier = new QPushButton("+", this);
    buttonEarlier->setToolTip(tr("Earlier"));
    connect(buttonEarlier, SIGNAL(clicked()), this, SLOT(onEarlier()));
    addScrollBarWidget(buttonEarlier, Qt::AlignTop);

    QPushButton* buttonLater = new QPushButton("+", this);
    buttonLater->setToolTip(tr("Later"));
    connect(buttonLater, SIGNAL(clicked()), this, SLOT(onLater()));
    addScrollBarWidget(buttonLater, Qt::AlignBottom);

    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    QFont mono("Monospace");
    horizontalHeader()->setResizeMode(QHeaderView::Interactive);
    horizontalHeader()->setMovable(true);
    verticalHeader()->setFont(mono);
    verticalHeader()->setDefaultSectionSize(20);

    horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(horizontalHeader(), SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(onHorizontalHeaderContextMenu(const QPoint&)));
    connect(horizontalHeader(), SIGNAL(sectionMoved(int, int, int)),
            this, SLOT(onHorizontalHeaderSectionMoved(int, int, int)));
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
    BOOST_FOREACH(const Column& c, mColumns)
        newModel->addColumn(makeColumn(c));

    mTableModel = newModel;
    setModel(mTableModel.get());

    horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

ObsColumnPtr DataList::makeColumn(const Column& c)
{
    boost::posix_time::time_duration toff = boost::posix_time::hours(c.timeOffset);

    if (c.type == MODEL) {
        ModelColumnPtr mc = ColumnFactory::columnForSensor(mMA, c.sensor, mTimeLimits);
        mc->setTimeOffset(toff);
        return mc;
    } else {
        ColumnFactory::DisplayType cdt = ColumnFactory::NEW_CORRECTED;
        if (c.type == ORIGINAL)
            cdt = ColumnFactory::ORIGINAL;
        else if (c.type == FLAGS)
            cdt = ColumnFactory::NEW_CONTROLINFO;
        DataColumnPtr dc = ColumnFactory::columnForSensor(mDA, c.sensor, mTimeLimits, cdt);
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

    const QModelIndex& currentIdx = currentIndex();
    QItemSelection selection;
    bool scroll = (not idxs.empty());
    BOOST_FOREACH(const QModelIndex idx, idxs) {
        selection.select(idx, idx);
        if (idx == currentIdx)
            scroll = false;
    }
    if (scroll) {
        scrollTo(idxs.front(), QAbstractItemView::PositionAtCenter);
        scrollTo(idxs.back(), QAbstractItemView::PositionAtCenter);
    }
    selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
}

void DataList::currentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    METLIBS_LOG_SCOPE();
    QTableView::currentChanged(current, previous);
    if (current.isValid()) {
        const SensorTime st = mTableModel->findSensorTime(current);
        METLIBS_LOG_DEBUG(LOGVAL(st));
        if (st.valid() and not eq_SensorTime()(mSensorTime, st)) {
            mSensorTime = st;
            /*emit*/ signalNavigateTo(st);
        }
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

    int column = horizontalHeader()->logicalIndexAt(pos);
    if (chosen == actionAdd) {
        QDialog d(this);
        Ui::DataListAddColumn ui;
        ui.setupUi(&d);
        ui.textStation->setText("17980");
        ui.radioCorrected->setChecked(true);
        ui.textParam->setText("105");
        ui.comboType->addItems(QStringList()  << "4" << "330"<< "504" << "514" << "-4" << "-514");
        ui.comboType->setCurrentIndex(0);

        if (d.exec() != QDialog::Accepted)
            return;

        bool ok;
        int stationId = ui.textStation->text().toInt(&ok);
        if (not ok)
            return;

        int paramId = ui.textParam->text().toInt(&ok);
        if (not ok)
            return;

        int typeId = ui.comboType->currentText().toInt(&ok);
        if (not ok)
            return;

        Column c;
        c.sensor = Sensor(stationId, paramId, 0, 0, typeId);
        c.type = CORRECTED;
        if (ui.radioOriginal->isChecked())
            c.type = ORIGINAL;
        else if (ui.radioFlags->isChecked())
            c.type = FLAGS;
        if (ui.radioModel->isChecked())
            c.type = MODEL;
        c.timeOffset = ui.spinTimeOffset->value();

        mColumns.insert(mColumns.begin() + column, c);
        mTableModel->insertColumn(column, makeColumn(c));
        horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
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
    horizontalHeader()->moveSection(to, from);
    mTableModel->moveColumn(from, to); // move back and switch model columns instead

    const Column c = mColumns[from];
    mColumns.erase(mColumns.begin() + from);
    mColumns.insert(mColumns.begin() + to, c);

    horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

std::string DataList::changes()
{
    METLIBS_LOG_SCOPE();

    Columns_t reordered;
    for(unsigned int visual=0; visual<mColumns.size(); ++visual) {
        const int logical = horizontalHeader()->logicalIndex(visual);
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
