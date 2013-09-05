
#include "DataList.hh"

#include <QtGui/QApplication>
#include <QtGui/QFont>
#include <QtGui/QHeaderView>

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
    const QModelIndex idx = mTableModel->findIndex(st);
    if (idx.isValid()) {
        scrollTo(idx);
        QItemSelection selection;
        selection.select(idx, idx);
        selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
    }
}
