#include "StationOrderedMissingDataModel.hh"
#include <QStandardItem>
#include <QList>
#include <QDebug>
#include <QDate>
#include <boost/foreach.hpp>
#include <boost/date_time.hpp>
#include <string>
#include <sstream>


namespace
{
std::string getDateRangeString_( const boost::gregorian::date & from, const boost::gregorian::date & to )
{
  std::string ret = to_simple_string(from);
  if ( from != to )
    ret += std::string( " - " ) + to_simple_string(to);
  return ret;
}


std::string dateList(const std::vector<boost::gregorian::date> & dates)
{
    typedef std::vector<boost::gregorian::date>::size_type index;

    if ( dates.empty() )
      return "";

    std::ostringstream ss;

    boost::gregorian::date current = dates.front();
    index pos = 0;
    for ( index i = 1; i < dates.size(); ++ i ) {
      boost::gregorian::date tmp( current );
      tmp += boost::gregorian::days(i - pos);
      if ( dates[i] != tmp ) {
        ss << getDateRangeString_( current, dates[ i -1 ] ) << ", ";
        current = dates[ i ];
        pos = i;
      }
    }
    ss << getDateRangeString_( current, dates[ dates.size() -1 ] );

    return ss.str();
}

class StationIdStandardItem : public QStandardItem
{
public:
    StationIdStandardItem(int value) :
        QStandardItem(QString::number(value)),
        value_(value)
    {
    }

    virtual bool operator < (const QStandardItem & other) const
    {
        const StationIdStandardItem * idItem = dynamic_cast<const StationIdStandardItem *>(& other);
        if ( ! idItem ) {
            qDebug() << "ERROR: Trying to compare StationIdStandardItem with QStandardItem";
            return QStandardItem::operator <(other);
        }
        return value_ < idItem->value_;
    }

private:
    int value_;
};
}

StationOrderedMissingDataModel::StationOrderedMissingDataModel(const MissingList & missing, QObject *parent) :
    QStandardItemModel(parent)
{
    QStringList headers;
    headers.push_back("ID");
    headers.push_back("Name");
    headers.push_back("Dates missing");
    setHorizontalHeaderLabels(headers);

    std::map<kvalobs::kvStation, std::vector<boost::gregorian::date> > orderedMissingList;

    for ( MissingList::const_iterator it = missing.begin(); it != missing.end(); ++ it ) {
        const boost::gregorian::date & date = it->first;
        BOOST_FOREACH ( const kvalobs::kvStation & station, it->second ) {
            orderedMissingList[station].push_back(date);
        }
    }

    for ( std::map<kvalobs::kvStation, std::vector<boost::gregorian::date> >::const_iterator it = orderedMissingList.begin(); it != orderedMissingList.end(); ++ it ) {
        const kvalobs::kvStation & station = it->first;
        const std::vector<boost::gregorian::date> & dates = it->second;

        QList<QStandardItem *> row;
        row.push_back(new StationIdStandardItem(station.stationID()));
        row.push_back(new QStandardItem(QString(station.name().c_str())));

        QString dateText = QString::fromStdString(dateList(dates));
        QStandardItem * dateItem = new QStandardItem(dateText);
        const boost::gregorian::date & date = dates.back();
        dateItem->setData(QDate(date.year(), date.month(), date.day()));
        row.push_back(dateItem);

        appendRow(row);
    }
}
