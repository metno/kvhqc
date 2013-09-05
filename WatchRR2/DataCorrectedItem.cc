
#include "DataCorrectedItem.hh"

#include "Helpers.hh"
#include "KvMetaDataBuffer.hh"
#include "ObsColumn.hh"
#include "Tasks.hh"

#include <kvalobs/kvDataOperations.h>
#include <boost/bind.hpp>
#include <boost/make_shared.hpp>

#include <QtCore/QVariant>
#include <QtGui/QApplication>
#include <QtGui/QBrush>

#define NDEBUG
#include "w2debug.hh"

DataCorrectedItem::DataCorrectedItem(bool showNew, Code2TextPtr codes)
    : mCodes(codes)
    , mShowNew(showNew)
{
}

Qt::ItemFlags DataCorrectedItem::flags() const
{
    Qt::ItemFlags f = DataItem::flags();
    if (mShowNew)
        f |= Qt::ItemIsEditable;
    return f;
}

QVariant DataCorrectedItem::data(EditDataPtr obs, int role) const
{
    if (not obs)
        return QVariant();

    if (role == Qt::BackgroundRole) {
        if (mShowNew and obs->hasTasks())
            return QBrush(Qt::red);
    } else if (role == Qt::ForegroundRole) {
        if (((mShowNew and not obs->hasTasks()) or not mShowNew) and mCodes->isCode(getValue(obs)))
            return Qt::darkGray;
    } else if (role == Qt::FontRole) {
        QFont f;
        if (mShowNew and obs->modifiedCorrected())
            f.setBold(true);
        return f;
    } else if (role == Qt::ToolTipRole or role == Qt::StatusTipRole) {
        QString tip = mCodes->asTip(getValue(obs));
        return Helpers::appendText(tip, tasks::asText(obs->allTasks()));
    } else if (role == Qt::DisplayRole or role == Qt::EditRole) {
        return mCodes->asText(getValue(obs));
    } else if (role == Qt::TextAlignmentRole) {
        return Qt::AlignVCenter+(mCodes->isCode(getValue(obs)) ? Qt::AlignLeft : Qt::AlignRight);
    } else if (role == ObsColumn::ValueTypeRole or role == ObsColumn::TextCodesRole) {
        const QStringList allCodes = mCodes->allCodes();
        if (role == ObsColumn::TextCodesRole)
            return allCodes;
        int valueTypes = ObsColumn::Numerical;
        if (not allCodes.empty())
            valueTypes |= ObsColumn::TextCode;
        return valueTypes;
    } else if (role ==  ObsColumn::TextCodeExplanationsRole) {
        return mCodes->allExplanations();
    }
    return DataItem::data(obs, role);
}

bool DataCorrectedItem::setData(EditDataPtr obs, EditAccessPtr da, const SensorTime& st, const QVariant& value, int role)
{
    if (role != Qt::EditRole or not mShowNew)
        return false;

    try {
        const float newC = mCodes->fromText(value.toString());
        const bool reject = (newC == kvalobs::REJECTED);
        if (reject and not obs)
            return false;
        if (not KvMetaDataBuffer::instance()->checkPhysicalLimits(st.sensor.paramId, newC))
            return false;

        da->newVersion();
        if (not obs)
            obs = da->createE(st);

        if (reject)
            Helpers::reject(da->editor(obs));
        else
            Helpers::auto_correct(da->editor(obs), newC);
        return true;
    } catch (std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
}

QString DataCorrectedItem::description(bool mini) const
{
    if (mini)
        return qApp->translate("DataColumn", "corr");
    else
        return qApp->translate("DataColumn", "corrected");
}

float DataCorrectedItem::getValue(EditDataPtr obs) const
{
    if (not obs)
        return kvalobs::MISSING;
    if (not mShowNew)
        return obs->oldCorrected();
    return obs->corrected();
}
