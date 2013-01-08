
#ifndef MODELCOLUMN_HH
#define MODELCOLUMN_HH 1

#include "Code2Text.hh"
#include "ModelAccess.hh"
#include "ObsColumn.hh"

class ModelColumn : public ObsColumn {
public:
    ModelColumn(ModelAccessPtr ma, const Sensor& sensor, const TimeRange& time);
    ~ModelColumn();

    void setHeaderShowStation(bool show)
        { mHeaderShowStation = show; }

    virtual Qt::ItemFlags flags(const timeutil::ptime& time) const;
    virtual QVariant data(const timeutil::ptime& time, int role) const;
    virtual bool setData(const timeutil::ptime& time, const QVariant& value, int role);
    virtual QVariant headerData(int role) const;

    virtual const boost::posix_time::time_duration& timeOffset() const
        { return mTimeOffset; }
    void setTimeOffset(const boost::posix_time::time_duration& timeOffset);

    void setCodes(Code2TextPtr codes);

protected:
    ModelDataPtr getModel(const timeutil::ptime& time) const;

    float getValue(const timeutil::ptime& time) const
        { return getValue(getModel(time)); }

    float getValue(ModelDataPtr mdl) const;

    virtual bool onModelDataChanged(ModelDataPtr obs);

protected:
    ModelAccessPtr mMA;
    Sensor mSensor;
    bool mHeaderShowStation;
    boost::posix_time::time_duration mTimeOffset;

    Code2TextPtr mCodes;

    typedef std::map<timeutil::ptime, ModelDataPtr> ModelCache_t;
    mutable ModelCache_t mModelCache;
};

typedef boost::shared_ptr<ModelColumn> ModelColumnPtr;

#endif // MODELCOLUMN_HH
