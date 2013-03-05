
#include "DataVxItem.hh"

#include "FlagChange.hh"
#include "Helpers.hh"
#include "ObsColumn.hh"
#include "Tasks.hh"

#include <QtCore/QStringList>
#include <QtCore/QVariant>
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
    {  0, "",   QT_TRANSLATE_NOOP("DataVxItem", "no data") },
    {  3, "RR", QT_TRANSLATE_NOOP("DataVxItem", "rain") },
    {  7, "RB", QT_TRANSLATE_NOOP("DataVxItem", "rain shower") },
    {  2, "SS", QT_TRANSLATE_NOOP("DataVxItem", "snow") },
    {  5, "SB", QT_TRANSLATE_NOOP("DataVxItem", "snow shower") },
    {  1, "SL", QT_TRANSLATE_NOOP("DataVxItem", "sleet") },
    {  4, "LB", QT_TRANSLATE_NOOP("DataVxItem", "sleet shower") },
    {  8, "YR", QT_TRANSLATE_NOOP("DataVxItem", "drizzle") },
    { 10, "HG", QT_TRANSLATE_NOOP("DataVxItem", "hail") },
    { 12, "DU", QT_TRANSLATE_NOOP("DataVxItem", "dew") },
    { 17, "RI", QT_TRANSLATE_NOOP("DataVxItem", "rime") },
    { 20, "TO", QT_TRANSLATE_NOOP("DataVxItem", "thunder") },
    { 28, "SF", QT_TRANSLATE_NOOP("DataVxItem", "snowflakes") },
    { -1, 0, 0 }
};
} // namespace anonymous

DataVxItem::DataVxItem(EditAccessPtr da)
    : mDA(da)
{
}

Qt::ItemFlags DataVxItem::flags() const
{
    return DataItem::flags() | Qt::ItemIsEditable;
}

QVariant DataVxItem::data(EditDataPtr obs1, int role) const
{
    if (obs1 and (role == Qt::DisplayRole or role == Qt::EditRole or role == Qt::ToolTipRole or role == Qt::StatusTipRole))
    {
        EditDataPtr obs2 = getObs2(obs1);

        Codes_t codes = getCodes(obs1, obs2);
        int i=0;
        for(; vxdata[i].code >= 0; ++i)
            if (codes.first == vxdata[i].code)
                break;
        if (vxdata[i].code < 0)
            return QVariant();

        if (role == Qt::DisplayRole or role == Qt::EditRole) {
            QString display = vxdata[i].metCode;
            if (codes.first != 0) {
                if (codes.second == 0)
                    display += QChar( 0xB0 );
                else if (codes.second == 2)
                    display += QChar( 0xB2 );
            }
            return display;
        } else {
            QString tooltip = qApp->translate("DataVxItem", vxdata[i].explain);
            if (codes.first != 0 and (codes.second == 0 or codes.second == 2)) {
                tooltip += " -- ";
                if (codes.second == 0)
                    tooltip += qApp->translate("DataVxItem", "weak");
                else
                    tooltip += qApp->translate("DataVxItem", "strong");
            }
            if (obs1->hasTasks())
                tooltip += "; " + tasks::asText(obs1->allTasks());
            return tooltip;
        }
        return QVariant();
    } else if (role == ObsColumn::ValueTypeRole) {
        return ObsColumn::TextCode;
    } else if (role == ObsColumn::TextCodesRole or role == ObsColumn::TextCodeExplanationsRole) {
        QStringList codes;
        for(int i=0; vxdata[i].code >= 0; ++i) {
            if (role == ObsColumn::TextCodesRole) {
                QString mc = vxdata[i].metCode;
                if (vxdata[i].code != 0) {
                    codes << (mc + QChar( 0xB0 ))
                          << (mc + " ")
                          << (mc + QChar( 0xB2 ));
                } else {
                    codes << "";
                }
            } else {
                QString tooltip = qApp->translate("DataVxItem", vxdata[i].explain);
                if (vxdata[i].code != 0) {
                    codes << (tooltip + " -- " + qApp->translate("DataVxItem", "weak"))
                          << tooltip
                          << (tooltip + " -- " + qApp->translate("DataVxItem", "strong"));
                } else {
                    codes << tooltip;
                }
            }
        }
        return codes;
    } else if (role == Qt::FontRole) {
        bool modified = (obs1 and obs1->modified());
        if (not modified) {
            EditDataPtr obs2 = getObs2(obs1);
            modified = (obs2 and obs2->modified());
        }
        QFont f;
        f.setBold(modified);
        return f;
    } else {
        return DataItem::data(obs1, role);
    }
}

bool DataVxItem::setData(EditDataPtr obs1, EditAccessPtr, const SensorTime& st, const QVariant& value, int role)
{
    if (role != Qt::EditRole)
        return false;

    LOG_SCOPE("DataVxItem");
    EditDataPtr obs2 = getObs2(obs1);
    const Codes_t oldCodes = getCodes(obs1, obs2);

    const QString v = value.toString();
    if (v == "") {
        mDA->pushUpdate();
        bool changed = false;
        if (obs1) {
            mDA->editor(obs1)->setCorrected(0);
            changed = true;
        }
        if (obs2 and oldCodes.second != 1) {
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

    LOG4SCOPE_DEBUG(DBG1(oldCodes.first) << DBG1(newCode1) << DBG1(oldCodes.second) << DBG1(newCode2));
    bool pushed = false;
    if (newCode1 != oldCodes.first) {
        mDA->pushUpdate();
        pushed = true;
        if (not obs1)
            obs1 = mDA->createE(st);
        Helpers::correct(mDA->editor(obs1), newCode1);
    }
    if (newCode2 != oldCodes.second) {
        if (not pushed)
            mDA->pushUpdate();
        if (not obs2) {
            SensorTime st2(st);
            st2.sensor.paramId += 1;
            obs2 = mDA->createE(st2);
        }
        Helpers::correct(mDA->editor(obs2), newCode2);
    }
    return true;
}


bool DataVxItem::matchSensor(const Sensor& sensorColumn, const Sensor& sensorObs) const
{
    return (eq_Sensor()(sensorColumn, sensorObs)
            or eq_Sensor()(getSensor2(sensorColumn), sensorObs));
        return true;
}

QString DataVxItem::description(bool mini) const
{
    return ""; // FIXME handle empty names in DataColumn::headerData
}

DataVxItem::Codes_t DataVxItem::getCodes(EditDataPtr obs1, EditDataPtr obs2) const
{
    return std::make_pair(obs1 ? static_cast<int>(obs1->corrected()) :  0,
                          obs2 ? static_cast<int>(obs2->corrected()) : -1);
}

EditDataPtr DataVxItem::getObs2(EditDataPtr obs1) const
{
    if (not obs1)
        return EditDataPtr();
    SensorTime st2(obs1->sensorTime());
    st2.sensor.paramId += 1;
    return mDA->findE(st2);
}

Sensor DataVxItem::getSensor2(const Sensor& sensor1) const
{
    Sensor sensor2(sensor1);
    sensor2.paramId += 1;
    return sensor2;
}
