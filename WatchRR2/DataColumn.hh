
#ifndef DATACOLUMN_HH
#define DATACOLUMN_HH 1

#include "Code2Text.hh"
#include "EditAccess.hh"
#include "ObsColumn.hh"

class DataColumn : public ObsColumn {
public:
    enum DisplayType { ORIGINAL,
                       OLD_CORRECTED, NEW_CORRECTED,
                       OLD_CONTROLINFO, NEW_CONTROLINFO,
                       MODEL,
                       N_DISPLAYTYPES };

    DataColumn(EditAccessPtr da, const Sensor& sensor, const TimeRange& time, DisplayType displayType);
    ~DataColumn();

    void setEditable(bool e)
        { mEditable = e; }
    void setHeaderShowStation(bool show)
        { mHeaderShowStation = show; }

    virtual Qt::ItemFlags flags(const timeutil::ptime& time) const;
    virtual QVariant data(const timeutil::ptime& time, int role) const;
    virtual bool setData(const timeutil::ptime& time, const QVariant& value, int role);
    virtual QVariant headerData(int role, bool verticalHeader) const;

    DisplayType getDisplayType() const
        { return mDisplayType; }

    virtual const boost::posix_time::time_duration& timeOffset() const
        { return mTimeOffset; }
    void setTimeOffset(const boost::posix_time::time_duration& timeOffset);

    void setCodes(Code2TextPtr codes);

protected:
    EditDataPtr getObs(const timeutil::ptime& time) const;

    float getValue(const timeutil::ptime& time) const
        { return getValue(getObs(time)); }

    float getValue(EditDataPtr obs) const;

    virtual bool onDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr obs);

protected:
    EditAccessPtr mDA;
    Sensor mSensor;
    TimeRange mTime;
    DisplayType mDisplayType;
    bool mEditable;
    bool mHeaderShowStation;
    boost::posix_time::time_duration mTimeOffset;

    Code2TextPtr mCodes;

    typedef std::map<timeutil::ptime, EditDataPtr> ObsCache_t;
    mutable ObsCache_t mObsCache;
};

typedef boost::shared_ptr<DataColumn> DataColumnPtr;

#endif // DATACOLUMN_HH
