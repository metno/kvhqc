
#include "ModelColumn.hh"

#include "ModelData.hh"
#include "SensorHeader.hh"

#include <kvalobs/kvDataOperations.h>

#include <boost/bind.hpp>
#include <boost/make_shared.hpp>

#define MILOGGER_CATEGORY "kvhqc.ModelColumn"
#include "HqcLogging.hh"

ModelColumn::ModelColumn(ModelAccessPtr ma, const Sensor& sensor, const TimeRange& time)
    : mMA(ma)
    , mSensor(sensor)
    , mHeaderShowStation(true)
    , mCodes(boost::make_shared<Code2Text>())
{
    if (not ModelAccess::isModelSensor(mSensor)) {
        METLIBS_LOG_WARN("not a model sensor: " << mSensor << ", adapting");
        mSensor = ModelAccess::makeModelSensor(mSensor);
    }
    mMA->modelDataChanged.connect(boost::bind(&ModelColumn::onModelDataChanged, this, _1));
}

ModelColumn::~ModelColumn()
{
    mMA->modelDataChanged.disconnect(boost::bind(&ModelColumn::onModelDataChanged, this, _1));
}

Qt::ItemFlags ModelColumn::flags(const timeutil::ptime& /*time*/) const
{
    return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
}

QVariant ModelColumn::data(const timeutil::ptime& time, int role) const
{
    ModelDataPtr mdl = getModel(time);
    if (not mdl)
        return QVariant();

    if (role == Qt::ToolTipRole or role == Qt::StatusTipRole) {
        return mCodes->asTip(getValue(mdl));
    } else if (role == Qt::DisplayRole or role == Qt::EditRole) {
        return mCodes->asText(getValue(mdl));
    } else if (role == Qt::TextAlignmentRole) {
        return Qt::AlignRight;
    }
    return QVariant();
}

bool ModelColumn::setData(const timeutil::ptime& /*time*/, const QVariant& /*value*/, int /*role*/)
{
    return false;
}

QVariant ModelColumn::headerData(Qt::Orientation orientation, int role) const
{
    SensorHeader sh(mSensor, mHeaderShowStation ? SensorHeader::ALWAYS : SensorHeader::TOOLTIP,
                    SensorHeader::ALWAYS, mTimeOffset.hours());
    return sh.modelHeader(orientation, role);
}

bool ModelColumn::onModelDataChanged(ModelDataPtr mdl)
{
    const SensorTime st(mdl->sensorTime());
    if (not eq_Sensor()(st.sensor, mSensor))
        return false;
    const timeutil::ptime timeo = st.time - mTimeOffset;
    ModelCache_t::iterator it = mModelCache.find(timeo);
    if (it == mModelCache.end())
        return false;
    
    mModelCache.erase(it);
    columnChanged(timeo, this);
    return true;
}

ModelDataPtr ModelColumn::getModel(const timeutil::ptime& time) const
{
    ModelCache_t::iterator it = mModelCache.find(time);
    ModelDataPtr mdl;
    if (it == mModelCache.end()) {
        mdl = mMA->find(SensorTime(mSensor, time + mTimeOffset));
        mModelCache[time] = mdl;
    } else {
        mdl = it->second;
    }
    return mdl;
}

float ModelColumn::getValue(ModelDataPtr mdl) const
{
    return mdl->value();
}

void ModelColumn::setTimeOffset(const boost::posix_time::time_duration& timeOffset)
{
    mTimeOffset = timeOffset;
}

Sensor ModelColumn::sensor() const
{
    return mSensor;
}

void ModelColumn::setCodes(Code2TextCPtr codes)
{
    mCodes = codes;
}
