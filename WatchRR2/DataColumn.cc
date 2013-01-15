
#include "DataColumn.hh"

#include "Helpers.hh"
#include "FlagChange.hh"
#include "Tasks.hh"

#include <kvalobs/kvDataOperations.h>
#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
#include <QtGui/QApplication>
#include <QtGui/QBrush>

#define NDEBUG
#include "w2debug.hh"

DataColumn::DataColumn(EditAccessPtr da, const Sensor& sensor, const TimeRange& t, DisplayType displayType)
    : mDA(da)
    , mSensor(sensor)
    , mTime(t)
    , mDisplayType(displayType)
    , mEditable(displayType == NEW_CORRECTED)
    , mHeaderShowStation(true)
    , mCodes(boost::make_shared<Code2Text>())
{
    mDA->addSubscription(ObsSubscription(mSensor.stationId, mTime));
    mDA->obsDataChanged.connect(boost::bind(&DataColumn::onDataChanged, this, _1, _2));
}

DataColumn::~DataColumn()
{
    mDA->obsDataChanged.disconnect(boost::bind(&DataColumn::onDataChanged, this, _1, _2));
    mDA->removeSubscription(ObsSubscription(mSensor.stationId, mTime));
}

Qt::ItemFlags DataColumn::flags(const timeutil::ptime& time) const
{
    Qt::ItemFlags f = Qt::ItemIsEnabled|Qt::ItemIsSelectable;
    EditDataPtr obs = getObs(time);
    if (mEditable and (mDisplayType == NEW_CORRECTED))
        f |= Qt::ItemIsEditable;
    return f;
}

static int extract_ui2(EditDataPtr obs)
{
    kvalobs::kvUseInfo ui;
    ui.setUseFlags(obs->controlinfo());
    return ui.flag(2);
}

QVariant DataColumn::data(const timeutil::ptime& time, int role) const
{
    EditDataPtr obs = getObs(time);
    if (not obs)
        return QVariant();

    if (role == Qt::BackgroundRole) {
        if (mDisplayType == NEW_CORRECTED) {
            if (obs->hasTasks())
                return QBrush(Qt::red);
        } else if (mDisplayType == ORIGINAL) {
            const int ui_2 = extract_ui2(obs);
            if (ui_2 == 1)      // probably ok
                return QBrush(QColor(0xFF, 0xFF, 0xF0)); // light yellow
            else if (ui_2 == 2) // probably wrong
                return QBrush(QColor(0xFF, 0xF0, 0xF0)); // light red
            else if (ui_2 == 9) // no quality information
                return QBrush(QColor(0xFF, 0xE0, 0xB0)); // light orange
            else if (ui_2 != 0) // wrong
                return QBrush(QColor(0xFF, 0xE0, 0xE0)); // light red
        }
    } else if (role == Qt::ForegroundRole) {
        if (((mDisplayType == NEW_CORRECTED and not obs->hasTasks())
             or mDisplayType == ORIGINAL or mDisplayType == OLD_CORRECTED)
            and mCodes->isCode(getValue(obs)))
        {
            return Qt::darkGray;
        }
    } else if (role == Qt::FontRole) {
        QFont f;
        if ((mDisplayType == NEW_CORRECTED and obs->modifiedCorrected())
            or (mDisplayType == NEW_CONTROLINFO and obs->modifiedControlinfo()))
        {
            f.setBold(true);
        }
        return f;
    } else if (role == Qt::ToolTipRole or role == Qt::StatusTipRole) {
        if (mDisplayType == OLD_CONTROLINFO or mDisplayType == NEW_CONTROLINFO ) {
            const kvalobs::kvControlInfo& ci = (mDisplayType == OLD_CONTROLINFO) ? obs->oldControlinfo() : obs->controlinfo();
            return Helpers::getFlagExplanation(ci);
        }
        QString tip;
        if (mDisplayType == ORIGINAL) {
            const int ui_2 = extract_ui2(obs);
            if (ui_2 == 3)
                Helpers::appendText(tip, qApp->translate("DataColumn", "surely wrong"));
            else if (ui_2 == 2)
                Helpers::appendText(tip, qApp->translate("DataColumn", "very suspicious (probably wrong)"));
            else if (ui_2 == 1)
                Helpers::appendText(tip, qApp->translate("DataColumn", "suspicious (probably ok)"));
            else if (ui_2 == 9)
                Helpers::appendText(tip, qApp->translate("DataColumn", "no quality info available"));
        }
        Helpers::appendText(tip, mCodes->asTip(getValue(obs)));
        return Helpers::appendText(tip, tasks::asText(obs->allTasks()));
    } else if (role == Qt::DisplayRole or role == Qt::EditRole) {
        if (mDisplayType == OLD_CONTROLINFO or mDisplayType == NEW_CONTROLINFO ) {
            const kvalobs::kvControlInfo& ci = (mDisplayType == OLD_CONTROLINFO) ? obs->oldControlinfo() : obs->controlinfo();
            return Helpers::getFlagText(ci);
        }
        return mCodes->asText(getValue(obs));
    } else if (role == ValueTypeRole) {
        return Numerical;
    } else if (role == TextCodesRole) {
        return QStringList();
    }
    return QVariant();
}

bool DataColumn::setData(const timeutil::ptime& time, const QVariant& value, int role)
{
    if (role != Qt::EditRole or mDisplayType != NEW_CORRECTED)
        return false;

    try {
        const float newC = mCodes->fromText(value.toString());
        EditDataPtr obs = getObs(time);
        const bool reject = (newC == kvalobs::REJECTED);
        if (reject and not obs)
            return false;
        mDA->pushUpdate();
        if (not obs)
            obs = mDA->createE(SensorTime(mSensor, time));

        if (reject)
            Helpers::reject(mDA->editor(obs));
        else
            Helpers::correct(mDA->editor(obs), newC);
        return true;
    } catch (std::string& error) {
        return false;
    }
}

QVariant DataColumn::headerData(int role, bool verticalHeader) const
{
    const int offset = mTimeOffset.hours();

    if (role == Qt::ToolTipRole) {
        const char* verboseDisplayTypes[N_DISPLAYTYPES] = {
            QT_TRANSLATE_NOOP("DataColumn", "original"),
            QT_TRANSLATE_NOOP("DataColumn", "corrected"),
            QT_TRANSLATE_NOOP("DataColumn", "corrected"),
            QT_TRANSLATE_NOOP("DataColumn", "controlflags"),
            QT_TRANSLATE_NOOP("DataColumn", "controlflags")
        };
        QString t = qApp->translate("DataColumn",
            QT_TRANSLATE_NOOP("DataColumn", "Station %1 Parameter %2 Level %3 Sensor %4 Type %5\n%6"))
            .arg(mSensor.stationId)
            .arg(Helpers::parameterName(mSensor.paramId))
            .arg(mSensor.level)
            .arg(mSensor.sensor)
            .arg(mSensor.typeId)
            .arg(qApp->translate("DataColumn", verboseDisplayTypes[mDisplayType]));

        if (offset != 0) {
            t += "\n";
            if (offset > 0)
                t += qApp->translate("DataColumn", QT_TRANSLATE_NOOP("DataColumn", "time offset +%1 hour(s)")).arg(offset);
            else
                t += qApp->translate("DataColumn", QT_TRANSLATE_NOOP("DataColumn", "time offset -%1 hour(s)")).arg(-offset);
        }
        return t;
    } else if (role == Qt::DisplayRole) {
        const char* displayTypes[N_DISPLAYTYPES] = {
            QT_TRANSLATE_NOOP("DataColumn", "orig"),
            QT_TRANSLATE_NOOP("DataColumn", "corr"),
            QT_TRANSLATE_NOOP("DataColumn", "corr"),
            QT_TRANSLATE_NOOP("DataColumn", "flags"),
            QT_TRANSLATE_NOOP("DataColumn", "flags")
        };
        
        const QString sep = verticalHeader ? "\n" : " ";
        QString h;
        if (mHeaderShowStation)
            h += QString("%1").arg(mSensor.stationId) + sep;
        
        h += Helpers::parameterName(mSensor.paramId)
            + sep
            + qApp->translate("DataColumn", displayTypes[mDisplayType]);

        if (offset != 0) {
            h += sep;
            if (offset > 0)
                h += qApp->translate("DataColumn", "+%1h").arg(offset);
            else
                h += qApp->translate("DataColumn", "-%1h").arg(-offset);
        }
        return h;
    }
    return QVariant();
}

bool DataColumn::onDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr obs)
{
    LOG_SCOPE();
    const SensorTime st(obs->sensorTime());
    if (not eq_Sensor()(st.sensor, mSensor))
        return false;
    const timeutil::ptime timeo = st.time - mTimeOffset;
    ObsCache_t::iterator it = mObsCache.find(timeo);
    if (it == mObsCache.end())
        return false;
    
    mObsCache.erase(it);
    columnChanged(timeo, this);
    return true;
}

EditDataPtr DataColumn::getObs(const timeutil::ptime& time) const
{
    ObsCache_t::iterator it = mObsCache.find(time);
    EditDataPtr obs;
    if (it == mObsCache.end()) {
        obs = mDA->findE(SensorTime(mSensor, time + mTimeOffset));
        mObsCache[time] = obs;
    } else {
        obs = it->second;
    }
    return obs;
}

float DataColumn::getValue(EditDataPtr obs) const
{
    float value = kvalobs::MISSING;
    if (obs) {
        switch(mDisplayType) {
        case ORIGINAL:      value = obs->original(); break;
        case OLD_CORRECTED: value = obs->oldCorrected(); break;
        case NEW_CORRECTED: value = obs->corrected(); break;
        default: break;
        }
    }
    return value;
}

void DataColumn::setTimeOffset(const boost::posix_time::time_duration& timeOffset)
{
    const TimeRange oldTime = mTime;
    mTime.shift(-mTimeOffset);
    mTimeOffset = timeOffset;
    mTime.shift(mTimeOffset);

    mDA->addSubscription(ObsSubscription(mSensor.stationId, mTime));
    mDA->removeSubscription(ObsSubscription(mSensor.stationId, oldTime));
}

void DataColumn::setCodes(boost::shared_ptr<Code2Text> codes)
{
    mCodes = codes;
}
