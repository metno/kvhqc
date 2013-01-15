
#include "DataControlinfoItem.hh"

#include "Helpers.hh"

#include <QtCore/QVariant>
#include <QtGui/QApplication>
#include <QtGui/QFont>

#include <boost/make_shared.hpp>

DataControlinfoItem::DataControlinfoItem(bool showNew)
    : mShowNew(showNew)
{
}

QVariant DataControlinfoItem::data(EditDataPtr obs, int role) const
{
    if (not obs)
        return QVariant();

    if (role == Qt::FontRole) {
        QFont f;
        if (mShowNew and obs->modifiedControlinfo())
            f.setBold(true);
        return f;
    } else if (role == Qt::ToolTipRole or role == Qt::StatusTipRole) {
        return Helpers::getFlagExplanation(getControlinfo(obs));
    } else if (role == Qt::DisplayRole or role == Qt::EditRole) {
        return Helpers::getFlagText(getControlinfo(obs));
    }
    return DataItem::data(obs, role);
}

QString DataControlinfoItem::description(bool mini) const
{
    if (mini)
        return qApp->translate("DataColumn", "flags");
    else
        return qApp->translate("DataColumn", "controlflags");
}

kvalobs::kvControlInfo DataControlinfoItem::getControlinfo(EditDataPtr obs) const
{
    if (not obs)
        throw std::runtime_error("no obs");
    if (mShowNew)
        return obs->controlinfo();
    else
        return obs->oldControlinfo();
}
