
#include "DataOriginalItem.hh"

#include "Helpers.hh"

#include <QtCore/QVariant>
#include <QtGui/QApplication>
#include <QtGui/QBrush>

#include <boost/make_shared.hpp>

#define MILOGGER_CATEGORY "kvhqc.DataOriginalItem"
#include "HqcLogging.hh"

DataOriginalItem::DataOriginalItem(Code2TextPtr codes)
    : mCodes(codes)
{
}

QVariant DataOriginalItem::data(EditDataPtr obs, int role) const
{
    if (not obs)
        return QVariant();

    if (role == Qt::BackgroundRole) {
        const int ui_2 = Helpers::extract_ui2(obs);
        if (ui_2 == 1)      // probably ok
            return QBrush(QColor(0xFF, 0xFF, 0xF0)); // light yellow
        else if (ui_2 == 2) // probably wrong
            return QBrush(QColor(0xFF, 0xF0, 0xF0)); // light red
        else if (ui_2 == 9) // no quality information
            return QBrush(QColor(0xFF, 0xE0, 0xB0)); // light orange
        else if (ui_2 != 0) // wrong
            return QBrush(QColor(0xFF, 0xE0, 0xE0)); // light red
    } else if (role == Qt::ForegroundRole) {
        if (mCodes->isCode(getValue(obs)))
            return Qt::darkGray;
    } else if (role == Qt::ToolTipRole or role == Qt::StatusTipRole) {
        QString tip;
        const int ui_2 = Helpers::extract_ui2(obs);
        if (ui_2 == 3)
            Helpers::appendText(tip, qApp->translate("DataOriginalItem", "surely wrong"));
        else if (ui_2 == 2)
            Helpers::appendText(tip, qApp->translate("DataOriginalItem", "very suspicious (probably wrong)"));
        else if (ui_2 == 1)
            Helpers::appendText(tip, qApp->translate("DataOriginalItem", "suspicious (probably ok)"));
        else if (ui_2 == 9)
            Helpers::appendText(tip, qApp->translate("DataOriginalItem", "no quality info available"));
        return Helpers::appendText(tip, mCodes->asTip(getValue(obs)));
    } else if (role == Qt::DisplayRole or role == Qt::EditRole) {
        return mCodes->asText(getValue(obs));
    } else if (role == Qt::TextAlignmentRole) {
        return Qt::AlignVCenter+(mCodes->isCode(getValue(obs)) ? Qt::AlignLeft : Qt::AlignRight);
    }
    return DataItem::data(obs, role);
}

QString DataOriginalItem::description(bool mini) const
{
    if (mini)
        return qApp->translate("DataColumn", "orig");
    else
        return qApp->translate("DataColumn", "original");
}

float DataOriginalItem::getValue(EditDataPtr obs) const
{
    if (not obs)
        return kvalobs::MISSING;
    return obs->original();
}
