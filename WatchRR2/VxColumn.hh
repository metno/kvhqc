
#ifndef VXCOLUMN_HH
#define VXCOLUMN_HH 1

#include "DataColumn.hh"

class VxColumn : public DataColumn {
public:
    VxColumn(EditAccessPtr kda, const Sensor& sensor, const TimeRange& time, DisplayType displayType);

    virtual QVariant data(const timeutil::ptime& time, int role) const;
    virtual bool setData(const timeutil::ptime& time, const QVariant& value, int role);

protected:
    virtual bool onDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr obs);

private:
    EditDataPtr getObs2(const timeutil::ptime& time) const;

private:
    Sensor mSensor2;
    mutable ObsCache_t mObsCache2;
};

#endif // VXCOLUMN_HH
