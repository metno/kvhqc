
#include "TimeHeader.hh"

#include <QtCore/QCoreApplication>

namespace /* anonymous */ {
const char* weekdays[7] = {
    QT_TRANSLATE_NOOP("TimeHeader", "Sun"),
    QT_TRANSLATE_NOOP("TimeHeader", "Mon"),
    QT_TRANSLATE_NOOP("TimeHeader", "Tue"),
    QT_TRANSLATE_NOOP("TimeHeader", "Wed"),
    QT_TRANSLATE_NOOP("TimeHeader", "Thu"),
    QT_TRANSLATE_NOOP("TimeHeader", "Fri"),
    QT_TRANSLATE_NOOP("TimeHeader", "Sat")
};
} // namespace anonymous

QVariant TimeHeader::headerData(const timeutil::ptime& time, Qt::Orientation orientation, int role)
{
    if (role == Qt::DisplayRole or role == Qt::ToolTipRole) {
        const boost::gregorian::greg_weekday wd = time.date().day_of_week();
        const QString weekday = qApp->translate("TimeHeader", weekdays[wd]);
        if (role == Qt::DisplayRole) {
            QString header = "";
            if (orientation == Qt::Vertical)
                header = weekday + " ";
            header += QString("%1/%2")
                .arg(time.date().day(), 2)
                .arg(time.date().month(), 2);
            return header;
        } else if (role == Qt::ToolTipRole) {
            return QString("%1, %2")
                .arg(weekday)
                .arg(QString::fromStdString(timeutil::to_iso_extended_string(time)));
        }
    }
    return QVariant();
}
