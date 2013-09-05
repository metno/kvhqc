
#include "DataList.hh"

#include <QtGui/QApplication>
#include <QtGui/QFont>
#include <QtGui/QHeaderView>

DataList::DataList(QWidget* parent)
    : QTableView(parent)
{
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
