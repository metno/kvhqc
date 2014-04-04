
#include "DataControlinfoItem.hh"

#include "KvHelpers.hh"

#include <QtCore/QVariant>
#include <QtGui/QApplication>
#include <QtGui/QFont>

#include <boost/make_shared.hpp>

DataControlinfoItem::DataControlinfoItem()
{
}

QVariant DataControlinfoItem::data(ObsData_p obs, const SensorTime& st, int role) const
{
  if (not obs)
    return QVariant();

  if (role == Qt::FontRole) {
    QFont f;
#if 0
    if (obs->modifiedControlinfo())
      f.setBold(true);
    return f;
#endif
  } else if (role == Qt::ToolTipRole or role == Qt::StatusTipRole) {
    return Helpers::getFlagExplanation(getControlinfo(obs));
  } else if (role == Qt::DisplayRole or role == Qt::EditRole) {
    return Helpers::getFlagText(getControlinfo(obs));
  }
  return DataItem::data(obs, st, role);
}

QString DataControlinfoItem::description(bool mini) const
{
  if (mini)
    return qApp->translate("DataColumn", "flags");
  else
    return qApp->translate("DataColumn", "controlflags");
}

const kvalobs::kvControlInfo& DataControlinfoItem::getControlinfo(ObsData_p obs) const
{
  if (not obs)
    throw std::runtime_error("no obs");
  return obs->controlinfo();
}
