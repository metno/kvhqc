
#include "StationSelection.hh"

#include "BusyIndicator.h"
#include "KvMetaDataBuffer.hh"

#include "ui_stationselection.h"

#include <kvalobs/kvObsPgm.h>

#include <boost/foreach.hpp>

#define MILOGGER_CATEGORY "kvhqc.StationSelection"
#define M_TIME 1
#include "HqcLogging.hh"

StationTable::StationTable(QWidget* parent)
    : Q3Table(0, 6, parent)
{
    setSorting( true );

    horizontalHeader()->setLabel( 0, tr( "Stnr" ) );
    horizontalHeader()->setLabel( 1, tr( "Name" ) );
    horizontalHeader()->setLabel( 2, tr( "HOH" ) );
    horizontalHeader()->setLabel( 3, tr( "County" ) );
    horizontalHeader()->setLabel( 4, tr( "Commune" ) );
    horizontalHeader()->setLabel( 5, tr( "Pri" ) );
}

void StationTable::setData(const listStat_l& listStat,
                           const QStringList& counties)
{
    METLIBS_LOG_TIME();
    METLIBS_LOG_DEBUG(LOGVAL(counties.join(";")));
    BusyIndicator busy;
    setNumRows(listStat.size());

    const bool allCounties = (counties.contains("ALL"));

    { // pre-fetching obs_pgm is a faster than fetching one-by-one
      std::set<long> stationids;
      BOOST_FOREACH(const listStat_t& s, listStat) {
        if (not (allCounties or counties.contains(QString::fromStdString(s.fylke))))
            continue;

        stationids.insert(s.stationid);
      }
      KvMetaDataBuffer::instance()->findObsPgm(stationids);
    }

    int stInd = 0;
    BOOST_FOREACH(const listStat_t& s, listStat) {
        METLIBS_LOG_DEBUG(LOGVAL(s.stationid) << LOGVAL(s.fylke) << LOGVAL(s.wmonr) << LOGVAL(s.pri));
        const QString prty = (s.pri > 0) ? QString("PRI%1").arg(s.pri) : QString();

        if (not (allCounties or counties.contains(QString::fromStdString(s.fylke))))
            continue;

        const std::list<kvalobs::kvObsPgm>& obsPgmList = KvMetaDataBuffer::instance()->findObsPgm(s.stationid);
        std::set<int> typeIDs;
        BOOST_FOREACH(const kvalobs::kvObsPgm& op, obsPgmList)
            typeIDs.insert(op.typeID());

        QString strEnv;
        StTableItem* stNum = new StTableItem(this, Q3TableItem::Never, QString::number(s.stationid));
        setItem(stInd, 0, stNum);
        StTableItem* stName = new StTableItem(this, Q3TableItem::Never, QString::fromStdString(s.name));
        setItem(stInd, 1, stName);
        StTableItem* stHoh = new StTableItem(this, Q3TableItem::Never, QString::number(s.altitude, 'f', 0));
        setItem(stInd, 2, stHoh);
        StTableItem* stFylke = new StTableItem(this, Q3TableItem::Never, QString::fromStdString(s.fylke));
        setItem(stInd, 3, stFylke);
        StTableItem* stKommune = new StTableItem(this, Q3TableItem::Never, QString::fromStdString(s.kommune));
        setItem(stInd, 4, stKommune);
        StTableItem* stPrior = new StTableItem(this, Q3TableItem::Never, prty);
        setItem(stInd, 5, stPrior);
        stInd++;
    }
    setNumRows(stInd);

    adjustColumn( 0 );
    adjustColumn( 1 );
    adjustColumn( 2 );
    adjustColumn( 3 );
    adjustColumn( 4 );
    adjustColumn( 5 );
}

void StationTable::sortColumn( int col, bool ascending, bool /*wholeRows*/ ) {
    Q3Table::sortColumn( col, ascending, true );
}

// ########################################################################
// ########################################################################
// ########################################################################

StationSelection::StationSelection(const listStat_l& listStat,
                                   const QStringList& counties,
                                   QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::StationSelectionDialog)
{
    ui->setupUi(this);

    connect(ui->selectionOK, SIGNAL(clicked()), this, SLOT(hide()));
    connect(ui->selectAllStations, SIGNAL(clicked()),SLOT(doSelectAllStations()));

    ui->stationTable->setData(listStat, counties);
    connect(ui->stationTable, SIGNAL(currentChanged(int, int)),
            this, SLOT(tableCellClicked(int, int)));
}

void StationSelection::tableCellClicked() {
}
void StationSelection::tableCellClicked(int row,
					int /*col*/,
					int /*button*/,
					const QPoint& /*mousePos*/) {
    ui->stationTable->selectRow(row);
    selectOrDeselectStation(row);
}

void StationSelection::tableCellClicked(int row, int /*col*/) {
    ui->stationTable->selectRow(row);
    selectOrDeselectStation(row);
}

void StationSelection::selectOrDeselectStation(int row)
{
    Q3TableItem* tStationNumber = ui->stationTable->item( row, 0);
    Q3TableItem* tStationName = ui->stationTable->item( row, 1);
    QString station = tStationNumber->text() + "  " + tStationName->text();
    
    const int stationID = tStationNumber->text().toInt();
    std::set<int>::iterator it = mSelectedStations.find(stationID);
    
    if (it != mSelectedStations.end() ) {
        mSelectedStations.erase(it);
        /*emit*/ stationRemoved(station);
    } else {
        mSelectedStations.insert(stationID);
        /*emit*/ stationAppended(station);
    }
}

void StationSelection::doSelectAllStations()
{
    for(int row = 0; row < ui->stationTable->numRows(); row++) {
        Q3TableItem* tStationNumber = ui->stationTable->item( row, 0);
        
        const int stationID = tStationNumber->text().toInt();
        if (mSelectedStations.find(stationID) == mSelectedStations.end()) {
            mSelectedStations.insert(stationID);

            Q3TableItem* tStationName = ui->stationTable->item( row, 1);
            QString station = tStationNumber->text() + "  " + tStationName->text();
            /*emit*/ stationAppended(station);
        }
    }
}

std::vector<int> StationSelection::getSelectedStations()
{
    return std::vector<int>(mSelectedStations.begin(), mSelectedStations.end());
}

// ########################################################################
// ########################################################################
// ########################################################################

QString StTableItem::key() const {
    QString item;
    if (col() == 0 || col() == 2)
        item.sprintf("%08d",text().toInt());
    else
        item = text();
    return item;
}
