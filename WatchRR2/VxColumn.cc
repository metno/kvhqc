
#include "VxColumn.hh"

#include "FlagChange.hh"
#include "Helpers.hh"
#include "Tasks.hh"

#include <QtCore/QStringList>
#include <QtGui/QApplication>
#include <QtGui/QFont>

#define NDEBUG
#include "w2debug.hh"

namespace /* anonymous */ {
struct VxData {
    int code;
    const char* metCode;
    const char* explain;
} vxdata[] = {
    {  0, "",   QT_TRANSLATE_NOOP("VxColumn", "no data") },
    {  3, "RR", QT_TRANSLATE_NOOP("VxColumn", "rain") },
    {  7, "RB", QT_TRANSLATE_NOOP("VxColumn", "rain shower") },
    {  2, "SS", QT_TRANSLATE_NOOP("VxColumn", "snow") },
    {  5, "SB", QT_TRANSLATE_NOOP("VxColumn", "snow shower") },
    {  1, "SL", QT_TRANSLATE_NOOP("VxColumn", "sleet") },
    {  4, "LB", QT_TRANSLATE_NOOP("VxColumn", "sleet shower") },
    {  8, "YR", QT_TRANSLATE_NOOP("VxColumn", "drizzle") },
    { 10, "HG", QT_TRANSLATE_NOOP("VxColumn", "hail") },
    { 12, "DU", QT_TRANSLATE_NOOP("VxColumn", "dew") },
    { 17, "RI", QT_TRANSLATE_NOOP("VxColumn", "rime") },
    { 20, "TO", QT_TRANSLATE_NOOP("VxColumn", "thunder") },
    { 28, "SF", QT_TRANSLATE_NOOP("VxColumn", "snowflakes") },
    { -1, 0, 0 }
};
} // namespace anonymous

VxColumn::VxColumn(EditAccessPtr da, const Sensor& sensor, DisplayType displayType)
    : DataColumn(da, sensor, displayType)
    , mSensor2(sensor)
{
    mSensor2.paramId += 1;
}

QVariant VxColumn::data(const timeutil::ptime& time, int role) const
{
    EditDataPtr obs1 = getObs(time), obs2 = mDA->findE(SensorTime(mSensor2, time + mTimeOffset));

    if (obs1 and (role == Qt::DisplayRole or role == Qt::EditRole or role == Qt::ToolTipRole or role == Qt::StatusTipRole))
    {
        int code1, code2 = 1;
        switch(mDisplayType) {
        case ORIGINAL: {
            code1 = static_cast<int>(obs1->original());
            if (obs2)
                code2 = static_cast<int>(obs2->original());
            break; }
        case OLD_CORRECTED: {
            code1 = static_cast<int>(obs1->oldCorrected());
            if (obs2)
                code2 = static_cast<int>(obs2->oldCorrected());
            break; }
        case NEW_CORRECTED: {
            code1 = static_cast<int>(obs1->corrected());
            if (obs2)
                code2 = static_cast<int>(obs2->corrected());
            break; }
        default:
            return DataColumn::data(time, role);
        }
        int i=0;
        for(; vxdata[i].code >= 0; ++i)
            if (code1 == vxdata[i].code)
                break;
        if (vxdata[i].code < 0)
            return QVariant();

        if (role == Qt::DisplayRole or role == Qt::EditRole) {
            QString display = vxdata[i].metCode;
            if (code1 != 0) {
                if (code2 == 0)
                    display += QChar( 0xB0 );
                else if (code2 == 2)
                    display += QChar( 0xB2 );
            }
            return display;
        } else {
            QString tooltip = qApp->translate("VxColumn", vxdata[i].explain);
            if (code1 != 0 and (code2 == 0 or code2 == 2)) {
                tooltip += " -- ";
                if (code2 == 0)
                    tooltip += qApp->translate("VxColumn", "weak");
                else
                    tooltip += qApp->translate("VxColumn", "strong");
            }
            if (obs1->hasTasks())
                tooltip += "; " + tasks::asText(obs1->allTasks());
            return tooltip;
        }
        return QVariant();
    } else if (obs1 and role == Qt::FontRole) {
        QFont f;
        if( (mDisplayType == NEW_CORRECTED or mDisplayType == NEW_CONTROLINFO)
            and (obs1->modified() or (obs2 and obs2->modified())) )
            f.setBold(true);
        return f;
    } else if (role == ValueTypeRole) {
        return TextCode;
    } else if (role == TextCodesRole) {
        QStringList codes;
        for(int i=0; vxdata[i].code >= 0; ++i) {
            QString mc = vxdata[i].metCode;
            if (vxdata[i].code != 0)
                codes << (mc + QChar( 0xB0 ))
                      << (mc + " ")
                      << (mc + QChar( 0xB2 ));
            else
                codes << "";
        }
        return codes;
    }
    return DataColumn::data(time, role);
}

bool VxColumn::setData(const timeutil::ptime& time, const QVariant& value, int role)
{
    if (role != Qt::EditRole or mDisplayType != NEW_CORRECTED)
        return false;

    const timeutil::ptime timeo = time + mTimeOffset;
    EditDataPtr obs1 = getObs(time), obs2 = mDA->findE(SensorTime(mSensor2, time + mTimeOffset));

    const int oldCode1 = obs1 ? static_cast<int>(obs1->corrected()) : 0,
        oldCode2 = obs2 ? static_cast<int>(obs2->corrected()) : -1;

    const QString v = value.toString();
    if (v == "") {
        mDA->pushUpdate();
        bool changed = false;
        if (obs1) {
            mDA->editor(obs1)->setCorrected(0);
            changed = true;
        }
        if (obs2 and oldCode2 != 1) {
            mDA->editor(obs2)->setCorrected(1);
            changed = true;
        }
        return changed;
    }

    if (v.length() != 3 and v.length() != 2)
        return false;
    const QString mc = v.left(2), level = v.mid(2);

    int i=1; // start at 1, skipping "no data"
    for(; vxdata[i].code >= 0; ++i) {
        if (mc == vxdata[i].metCode)
            break;
    }
    if (vxdata[i].code < 0)
        return false;
    const int newCode1 = vxdata[i].code;

    int newCode2;
    if (level == "" or level == " " or level == "1")
        newCode2 = 1;
    else if (level == QChar(0xB0) or level == "0")
        newCode2 = 0;
    else if (level == QChar(0xB2) or level == "2")
        newCode2 = 2;
    else
        return false;

    bool pushed = false;
    if (newCode1 != oldCode1) {
        mDA->pushUpdate();
        pushed = true;
        if (not obs1)
            obs1 = mDA->createE(SensorTime(mSensor, timeo));
        Helpers::correct(mDA->editor(obs1), newCode1);
    }
    if (newCode2 != oldCode2) {
        if (not pushed)
            mDA->pushUpdate();
        if (not obs2)
            obs2 = mDA->createE(SensorTime(mSensor2, timeo));
        Helpers::correct(mDA->editor(obs2), newCode2);
    }
    return true;
}

bool VxColumn::onDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr obs)
{
    LOG_SCOPE();
    bool changed2 = false;
    const SensorTime st2(obs->sensorTime());
    const timeutil::ptime timeo = st2.time - mTimeOffset;
    if (eq_Sensor()(st2.sensor, mSensor2)) {
        ObsCache_t::iterator it = mObsCache2.find(timeo);
        if (it != mObsCache2.end()) {
            mObsCache.erase(it);
            changed2 = true;
        }
    }

    const bool changed1 = DataColumn::onDataChanged(what, obs);
    if (changed2 and not changed1)
        columnChanged(timeo, this);
    return changed1 or changed2;
}

EditDataPtr VxColumn::getObs2(const timeutil::ptime& time) const
{
    ObsCache_t::iterator it = mObsCache2.find(time);
    EditDataPtr obs;
    if (it == mObsCache2.end()) {
        obs = mDA->findE(SensorTime(mSensor2, time + mTimeOffset));
        mObsCache2[time] = obs;
    } else {
        obs = it->second;
    }
    return obs;
}

