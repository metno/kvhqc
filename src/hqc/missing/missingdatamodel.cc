#include "missingdatamodel.hh"
#include <qdebug.h>
#include <QDate>
#include <QStandardItem>
#include <QStringList>
#include <boost/date_time.hpp>

MissingDataModel::MissingDataModel(const MissingList & missing, QObject *parent) :
    QStandardItemModel(parent)
{
    QStringList headers;
    headers.push_back("ID");
    headers.push_back("Name");
    setHorizontalHeaderLabels(headers);

    QStandardItem * root = invisibleRootItem();
    for ( MissingList::const_iterator it = missing.begin(); it != missing.end(); ++ it ) {
        const boost::gregorian::date & date = it->first;
        QDate d(date.year(), date.month(), date.day());

        QList<QStandardItem *> dateRow;
        QStandardItem * dateItem = new QStandardItem(d.toString());
        dateRow.push_back(dateItem);
        dateRow.push_back(new QStandardItem(""));

        root->appendRow(dateItem);

        for ( ExStationList::const_iterator stations = it->second.begin(); stations != it->second.end(); ++ stations ) {
            QList<QStandardItem *> missingRow;
            missingRow.push_back(new QStandardItem(QString::number(stations->stationID())));
            missingRow.push_back(new QStandardItem(QString(stations->name().c_str())));

            dateItem->appendRow(missingRow);
        }
    }
}

MissingDataModel::~MissingDataModel()
{
}
