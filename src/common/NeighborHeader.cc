
#include "NeighborHeader.hh"

#include "common/KvHelpers.hh"
#include "common/KvMetaDataBuffer.hh"
#include "util/Helpers.hh"

QVariant NeighborHeader::headerData(int ctr, int nbr, Qt::Orientation orientation, int role)
{
  if (role != Qt::DisplayRole)
    return QVariant();

  const QString sep = (orientation == Qt::Horizontal) ? "\n" : " ";
  try {
    const kvalobs::kvStation& sc = KvMetaDataBuffer::instance()->findStation(ctr);
    const kvalobs::kvStation& sn = KvMetaDataBuffer::instance()->findStation(nbr);

    QString header = QString::number(nbr) + sep, name = Helpers::stationName(sn);
    const int MAX_NAME = 5;
    if (name.length() > MAX_NAME+1)
      header += name.left(MAX_NAME) + QChar(0x2026);
    else
      header += name;
    if (ctr != nbr)
      header += sep + "[" + QString::number(Helpers::distance(sc.lon(), sc.lat(), sn.lon(), sn.lat()), 'f', 0) + "km]";
    return header;
  } catch (std::exception&) {
    return QString("?%1?").arg(nbr);
  }
}
