
#ifndef DATAITEM_HH
#define DATAITEM_HH 1

#include "EditAccess.hh"
#include "ObsColumn.hh"

class DataItem {
public:
    virtual ~DataItem();

    virtual Qt::ItemFlags flags() const;
    virtual QVariant data(EditDataPtr obs, int role) const;
    virtual bool setData(EditDataPtr obs, EditAccessPtr da, const SensorTime& st, const QVariant& value, int role);
    virtual QString description(bool mini) const = 0;
    virtual bool matchSensor(const Sensor& sensorColumn, const Sensor& sensorObs) const;
    virtual int type() const = 0;
};

typedef boost::shared_ptr<DataItem> DataItemPtr;

#endif // DATAITEM_HH
