
#ifndef DataList_hh
#define DataList_hh 1

#include "DataView.hh"
#include "ObsColumn.hh"

#include <QtGui/QTableView>

class QDomElement;
class DataListModel;

class DataList : public QTableView, public DataView
{   Q_OBJECT;
public:
    DataList(QWidget* parent=0);
    ~DataList();

    virtual void setSensorsAndTimes(const Sensors_t& sensors, const TimeRange& limits);

    virtual void navigateTo(const SensorTime&);

protected:
    virtual void currentChanged(const QModelIndex& current, const QModelIndex& previous);

private:
    enum ColumnType { CORRECTED, ORIGINAL, FLAGS, MODEL };
    struct Column {
        Sensor sensor;
        ColumnType type;
        int timeOffset;
        void toText(QDomElement& ce) const;
        void fromText(const QDomElement& ce);
    };
    struct eq_Column;
    typedef std::vector<Column> Columns_t;

private Q_SLOTS:
    void onEarlier();
    void onLater();
    void onHorizontalHeaderContextMenu(const QPoint& pos);
    void onHorizontalHeaderSectionMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex);

private:
    void updateModel();
    ObsColumnPtr makeColumn(const Column& c);
    std::string changes();
    void replay(const std::string& changes);

private:
    std::auto_ptr<DataListModel> mTableModel;
    SensorTime mSensorTime;

    TimeRange mTimeLimits, mOriginalTimeLimits;
    Columns_t mColumns,    mOriginalColumns;
};

#endif // DataList_hh
