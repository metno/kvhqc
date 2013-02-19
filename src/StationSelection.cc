
#include "StationSelection.hh"

#include "BusyIndicator.h"
#include "KvMetaDataBuffer.hh"

#include "ui_stationselection.h"

#include <kvalobs/kvObsPgm.h>

#include <boost/foreach.hpp>

#define NDEBUG
#include "qdebug.h"
#include "debug.hh"

StationTable::StationTable(QWidget* parent)
    : Q3Table(0, 7, parent)
{
    setSorting( true );

    horizontalHeader()->setLabel( 0, tr( "Stnr" ) );
    horizontalHeader()->setLabel( 1, tr( "Name" ) );
    horizontalHeader()->setLabel( 2, tr( "HOH" ) );
    horizontalHeader()->setLabel( 3, tr( "Type" ) );
    horizontalHeader()->setLabel( 4, tr( "County" ) );
    horizontalHeader()->setLabel( 5, tr( "Commune" ) );
    horizontalHeader()->setLabel( 6, tr( "Pri" ) );
}

void StationTable::setData(const listStat_l& listStat,
                           const QStringList& stationTypes,
                           const QStringList& counties,
			   bool web,
			   bool pri)
{
    BusyIndicator busy;
    setNumRows(listStat.size());

    DBGE(qDebug() << counties);

    int stInd = 0;
    BOOST_FOREACH(const listStat_t& s, listStat) {
        DBG(DBG1(s.stationid) << DBG1(s.fylke) << DBG1(s.wmonr) << DBG1(s.pri));
        bool webStat = (s.wmonr != "    ");
        bool priStat = (s.pri.substr(0, 3) == "PRI");
        QString prty;
        if (s.pri.size() >= 4 )
            prty = QString::fromStdString(s.pri.substr(3,1));

        if (not (counties.contains("ALL")
                 or counties.contains(QString::fromStdString(s.fylke))
                 or (webStat and web) or (priStat and pri)))
            continue;

        const std::list<kvalobs::kvObsPgm>& obsPgmList = KvMetaDataBuffer::instance()->findObsPgm(s.stationid);
        std::set<int> typeIDs;
        BOOST_FOREACH(const kvalobs::kvObsPgm& op, obsPgmList)
            typeIDs.insert(op.typeID());

        QString strEnv;
        if (stationTypes.contains("ALL") ) {
            strEnv = getEnvironment(s.environment, typeIDs);
        } else {
            if ((stationTypes.contains("AA") && ((s.environment == 8 && (typeIDs.count(3)  || typeIDs.count(311))) || typeIDs.count(330) || typeIDs.count(342))) ) strEnv += "AA";
            if ((stationTypes.contains("AF") && s.environment == 1 && typeIDs.count(311)) )  strEnv += "AF";
            if ((stationTypes.contains("AL") && s.environment == 2 && typeIDs.count(3)) ) strEnv += "AL";
            if ((stationTypes.contains("AV") && s.environment == 12 && typeIDs.count(3)) )  strEnv += "AV";
            if ((stationTypes.contains("AO") && typeIDs.count(410)) )  strEnv += "AO";
            if ((stationTypes.contains("MV") && s.environment == 7 && typeIDs.count(11)) ) strEnv += "MV";
            if ((stationTypes.contains("MP") && s.environment == 5 && typeIDs.count(11)) ) strEnv += "MP";
            if ((stationTypes.contains("MM") && s.environment == 4 && typeIDs.count(11)) ) strEnv += "MM";
            if ((stationTypes.contains("MS") && s.environment == 6 && typeIDs.count(11)) ) strEnv += "MS";
            if ((stationTypes.contains("P")  && (typeIDs.count(4) || typeIDs.count(404))) ) strEnv += "P";
            if ((stationTypes.contains("PT") && (typeIDs.count(4) || typeIDs.count(404))) ) strEnv += "PT";
            if ((stationTypes.contains("NS") && typeIDs.count(302)) )  strEnv += "NS";
            if ((stationTypes.contains("ND") && s.environment == 9 && typeIDs.count(402)) ) strEnv += "ND";
            if ((stationTypes.contains("NO") && s.environment == 10 && typeIDs.count(402)) ) strEnv += "NO";
            if ((stationTypes.contains("VS") && (typeIDs.count(1) || typeIDs.count(6) || typeIDs.count(312))) ) strEnv += "VS";
            if ((stationTypes.contains("VK") && s.environment == 3 && typeIDs.count(412)) ) strEnv += "VK";
            if ((stationTypes.contains("VM") && (typeIDs.count(306) || typeIDs.count(308))) ) strEnv += "VM";
            if ((stationTypes.contains("FM") && (typeIDs.count(2))) ) strEnv += "FM";
        }
        if (not strEnv.isEmpty() ) {
            StTableItem* stNum = new StTableItem(this, Q3TableItem::Never, QString::number(s.stationid));
            setItem(stInd, 0, stNum);
            StTableItem* stName = new StTableItem(this, Q3TableItem::Never, QString::fromStdString(s.name));
            setItem(stInd, 1, stName);
            StTableItem* stHoh = new StTableItem(this, Q3TableItem::Never, QString::number(s.altitude, 'f', 0));
            setItem(stInd, 2, stHoh);
            StTableItem* stType = new StTableItem(this, Q3TableItem::Never, strEnv);
            setItem(stInd, 3, stType);
            StTableItem* stFylke = new StTableItem(this, Q3TableItem::Never, QString::fromStdString(s.fylke));
            setItem(stInd, 4, stFylke);
            StTableItem* stKommune = new StTableItem(this, Q3TableItem::Never, QString::fromStdString(s.kommune));
            setItem(stInd, 5, stKommune);
            StTableItem* stPrior = new StTableItem(this, Q3TableItem::Never, prty);
            setItem(stInd, 6, stPrior);
            stInd++;
        }
    }
    setNumRows(stInd);

    adjustColumn( 0 );
    adjustColumn( 1 );
    adjustColumn( 2 );
    adjustColumn( 3 );
    adjustColumn( 4 );
    adjustColumn( 5 );
    adjustColumn( 6 );
    if (pri)
        sortColumn(6, true, true);
    else
        hideColumn(6);
}

QString StationTable::getEnvironment(const int envID, const std::set<int>& typeIDs) {
    QString env;
    if (envID == 1 && typeIDs.count(311) )
        env = "AF";
    else if (envID == 2 && typeIDs.count(3) )
        env = "AL";
    else if (envID == 4 && typeIDs.count(11) )
        env = "MM";
    else if (envID == 5 && typeIDs.count(11) )
        env = "MP";
    else if (envID == 6 && typeIDs.count(11) )
        env = "MS";
    else if (envID == 7 && typeIDs.count(11) )
        env = "MV";
    else if ((envID == 8 && (typeIDs.count(3)  || typeIDs.count(311))) || typeIDs.count(330) || typeIDs.count(342) )
        env = "AA";
    else if (envID == 9 && typeIDs.count(402) )
        env = "ND";
    else if (envID == 10 && typeIDs.count(402) )
        env = "NO";
    else if (typeIDs.count(302) )
        env = "NS";
    else if (typeIDs.count(410) )
        env = "AO";
    else if (typeIDs.count(4) || typeIDs.count(404) )
        env = "P,PT";
    else if (typeIDs.count(2) )
        env = "FM";
    else if (typeIDs.count(1) || typeIDs.count(6) || typeIDs.count(312) )
        env = "VS";
    else if (typeIDs.count(306) || typeIDs.count(308) )
        env = "VM";
    else if (envID == 11 )
        env = "TURISTFORENING";
    else if (envID == 12 && typeIDs.count(3) )
        env = "AV";
    else if (typeIDs.count(412) )
        env = "VK";
    //  else if (typeIDs.count(503) )
    else if (typeIDs.count(502) || typeIDs.count(503) || typeIDs.count(504) || typeIDs.count(505) || typeIDs.count(506) || typeIDs.count(514) )
        env = "X";
    return env;
}


void StationTable::sortColumn( int col, bool ascending, bool /*wholeRows*/ ) {
    Q3Table::sortColumn( col, ascending, true );
}

// ########################################################################
// ########################################################################
// ########################################################################

StationSelection::StationSelection(const listStat_l& listStat,
                                   const QStringList& stationTypes,
                                   const QStringList& counties,
				   bool web,
				   bool pri,
                                   QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::StationSelectionDialog)
{
    ui->setupUi(this);

    connect(ui->selectionOK, SIGNAL(clicked()), this, SLOT(hide()));
    connect(ui->selectAllStations, SIGNAL(clicked()),SLOT(doSelectAllStations()));

    ui->stationTable->setData(listStat, stationTypes, counties, web, pri);
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
