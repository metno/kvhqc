
#ifndef DataList_hh
#define DataList_hh 1

#include "DataView.hh"

#include <QtGui/QTableView>

class DataListModel;

class DataList : public QTableView, public DataView
{   Q_OBJECT;
public:
    DataList(QWidget* parent=0);
    ~DataList();

    void setSensorsAndTimes(const Sensors_t& sensors, const TimeRange& limits);

    void navigateTo(const SensorTime&);

protected:
    virtual void currentChanged(const QModelIndex& current, const QModelIndex& previous);

private:
    std::auto_ptr<DataListModel> mTableModel;
    SensorTime mSensorTime;
};

#endif // DataList_hh
