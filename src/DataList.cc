
#include "DataList.hh"
#include "DataListModel.hh"

#include <QtGui/QFont>
#include <QtGui/QHeaderView>

#include <boost/foreach.hpp>

#define NDEBUG
#include "debug.hh"

DataList::DataList(QWidget* parent)
    : QTableView(parent)
{
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
}

DataList::~DataList()
{
}

void DataList::setSensorsAndTimes(const Sensors_t& sensors, const TimeRange& limits)
{
    DataView::setSensorsAndTimes(sensors, limits);

    mTableModel = std::auto_ptr<DataListModel>(new DataListModel(mDA, sensors, limits));
    setModel(mTableModel.get());

    QFont mono("Monospace");
    horizontalHeader()->setResizeMode(QHeaderView::Interactive);
    horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    verticalHeader()->setFont(mono);
    verticalHeader()->setDefaultSectionSize(20);
}

void DataList::navigateTo(const SensorTime& st)
{
    LOG_SCOPE("DataList");
    if (not st.valid() or eq_SensorTime()(mSensorTime, st))
        return;

    mSensorTime = st;
    LOG4SCOPE_DEBUG(DBG1(mSensorTime));

    const QModelIndexList idxs = mTableModel->findIndexes(st);
    QItemSelection selection;
    BOOST_FOREACH(const QModelIndex idx, idxs)
        selection.select(idx, idx);
    selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
    if (not idxs.empty())
        scrollTo(idxs.at(0));
}

void DataList::currentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    LOG_SCOPE("DataList");
    QTableView::currentChanged(current, previous);

    if (not current.isValid())
        return;

    const SensorTime st = mTableModel->findSensorTime(current);
    LOG4SCOPE_DEBUG(DBG1(st));
    if (st.valid())
        /*emit*/ signalNavigateTo(st);
}
