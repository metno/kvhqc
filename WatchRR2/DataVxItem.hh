
#ifndef DATAVXITEM_HH
#define DATAVXITEM_HH 1

#include "DataItem.hh"

class DataVxItem : public DataItem {
public:
    DataVxItem(EditAccessPtr da);

    virtual Qt::ItemFlags flags() const;
    virtual QVariant data(EditDataPtr obs, int role) const;
    virtual bool setData(EditDataPtr obs, EditAccessPtr da, const SensorTime& st, const QVariant& value, int role);
    virtual QString description(bool mini) const;
    virtual bool matchSensor(const Sensor& sensorColumn, const Sensor& sensorObs) const;
    virtual int type() const
    { return ObsColumn::NEW_CORRECTED; }

private:
    typedef std::pair<int,int> Codes_t;
    Codes_t getCodes(EditDataPtr obs1, EditDataPtr obs2) const;
    EditDataPtr getObs2(EditDataPtr obs1) const;
    Sensor getSensor2(const Sensor& sensor1) const;

private:
    EditAccessPtr mDA;
};

#endif // DATAVXITEM_HH
