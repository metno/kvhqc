
#include "DataList.hh"

#include <QtGui/QApplication>
#include <QtGui/QFont>
#include <QtGui/QHeaderView>

#include <boost/foreach.hpp>

DataList::DataList(QWidget* parent)
    : QTableView(parent)
{
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
}

DataList::~DataList()
{
}

void DataList::setSensorsAndTimes(EditAccessPtr eda, const DataListModel::Sensors_t& sensors, const TimeRange& limits)
{
    mTableModel = std::auto_ptr<DataListModel>(new DataListModel(eda, sensors, limits));
    setModel(mTableModel.get());

    QFont mono("Monospace");
    horizontalHeader()->setResizeMode(QHeaderView::Interactive);
    horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    verticalHeader()->setFont(mono);
    verticalHeader()->setDefaultSectionSize(20);
    qApp->processEvents();
}

void DataList::navigateTo(const SensorTime& st)
{
    const QModelIndexList idxs = mTableModel->findIndexes(st);
    QItemSelection selection;
    BOOST_FOREACH(const QModelIndex idx, idxs)
        selection.select(idx, idx);
    selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
    if (not idxs.empty())
        scrollTo(idxs.at(0));
}
