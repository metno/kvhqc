#include "StationOrderedMissingDataModel.hh"
#include <QStandardItem>
#include <QList>
#include <boost/foreach.hpp>
#include <boost/date_time.hpp>


namespace
{
QString dateList(const std::set<boost::gregorian::date> & dates)
{
    QString ret;
    BOOST_FOREACH(const boost::gregorian::date & date, dates) {
        std::string s_date = to_simple_string(date);
        if ( not ret.isEmpty() )
            ret += " ";
        ret += s_date.c_str();
    }
    return ret;
}
}

StationOrderedMissingDataModel::StationOrderedMissingDataModel(const MissingList & missing, QObject *parent) :
    QStandardItemModel(parent)
{
    QStringList headers;
    headers.push_back("ID");
    headers.push_back("Name");
    headers.push_back("Dates missing");
    setHorizontalHeaderLabels(headers);

    std::map<kvalobs::kvStation, std::set<boost::gregorian::date> > orderedMissingList;

    for ( MissingList::const_iterator it = missing.begin(); it != missing.end(); ++ it ) {
        const boost::gregorian::date & date = it->first;
        BOOST_FOREACH ( const kvalobs::kvStation & station, it->second ) {
            orderedMissingList[station].insert(date);
        }
    }

    for ( std::map<kvalobs::kvStation, std::set<boost::gregorian::date> >::const_iterator it = orderedMissingList.begin(); it != orderedMissingList.end(); ++ it ) {
        const kvalobs::kvStation & station = it->first;
        const std::set<boost::gregorian::date> & dates = it->second;

        QList<QStandardItem *> row;
        row.push_back(new QStandardItem(QString::number(station.stationID())));
        row.push_back(new QStandardItem(QString(station.name().c_str())));
        row.push_back(new QStandardItem(dateList(dates)));

        appendRow(row);
    }
}

//{
//    QStringList headers;
//    headers.push_back("ID");
//    headers.push_back("Name");
//    setHorizontalHeaderLabels(headers);

//    QStandardItem * root = invisibleRootItem();
//    for ( MissingList::const_iterator it = missing.begin(); it != missing.end(); ++ it ) {
//        const boost::gregorian::date & date = it->first;
//        QDate d(date.year(), date.month(), date.day());

//        QList<QStandardItem *> dateRow;
//        QStandardItem * dateItem = new QStandardItem(d.toString());
//        dateRow.push_back(dateItem);
//        dateRow.push_back(new QStandardItem(""));

//        root->appendRow(dateItem);

//        for ( ExStationList::const_iterator stations = it->second.begin(); stations != it->second.end(); ++ stations ) {
//            QList<QStandardItem *> missingRow;
//            missingRow.push_back(new QStandardItem(QString::number(stations->stationID())));
//            missingRow.push_back(new QStandardItem(QString(stations->name().c_str())));

//            dateItem->appendRow(missingRow);
//        }
//    }
//}
