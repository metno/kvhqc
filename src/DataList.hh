
#ifndef DataList_hh
#define DataList_hh 1

#include "DataListModel.hh"
#include "EditAccess.hh"
#include "ModelAccess.hh"

#include <QtGui/QTableView>

class DataList : public QTableView
{   Q_OBJECT;
public:
    DataList(QWidget* parent=0);
    ~DataList();

    void setSensorsAndTimes(EditAccessPtr eda, const DataListModel::Sensors_t& sensors, const TimeRange& limits);

private:
    std::auto_ptr<DataListModel> mTableModel;
};

#endif // DataList_hh
