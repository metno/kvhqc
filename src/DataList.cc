
#include "DataList.hh"

#include "ColumnFactory.hh"
#include "DataListModel.hh"

#include <QtGui/QFont>
#include <QtGui/QHeaderView>
#include <QtGui/QMenu>
#include <QtGui/QPushButton>

#include <boost/foreach.hpp>

#include "ui_dl_addcolumn.h"

#define NDEBUG
#include "debug.hh"

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
    LOG_SCOPE("DataList");

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
    LOG_SCOPE("DataList");
    if (not st.valid() or eq_SensorTime()(mSensorTime, st))
        return;

    mSensorTime = st;
    LOG4SCOPE_DEBUG(DBG1(mSensorTime));

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
    LOG_SCOPE("DataList");
    QTableView::currentChanged(current, previous);
    if (current.isValid()) {
        const SensorTime st = mTableModel->findSensorTime(current);
        LOG4SCOPE_DEBUG(DBG1(st));
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

    int logicalColumn = horizontalHeader()->logicalIndexAt(pos);
    if (chosen == actionAdd) {
        QDialog d(this);
        Ui::DataListAddColumn ui;
        ui.setupUi(&d);
        ui.textStation->setText("18700");
        ui.radioCorrected->setChecked(true);
        ui.textParam->setText("211");
        ui.comboType->addItems(QStringList() << "330" << "4" << "504");
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

        mColumns.insert(mColumns.begin() + logicalColumn, c);
        mTableModel->insertColumn(logicalColumn, makeColumn(c));
        horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    } else if (chosen == actionDel) {
        mColumns.erase(mColumns.begin() + logicalColumn);
        updateModel();
    }
}
