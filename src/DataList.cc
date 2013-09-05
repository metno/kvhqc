
#include "DataList.hh"

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
}
