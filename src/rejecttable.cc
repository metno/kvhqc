#include "rejecttable.h"
#include <iostream>
#include <QSizePolicy>

using namespace std;

RejectTable::RejectTable(vector<kvalobs::kvRejectdecode> rejList, QWidget* parent)
  : QTableWidget(3000,3,parent) {

  setWindowTitle("Rejected");
  setRowCount(rejList.size());
  setGeometry(10,10,1200,1200);
  setMinimumWidth(1000);
  setMinimumHeight(1000);
  QFont font("Courier", 9, QFont::DemiBold);
  setFont(font);  
  QStringList horizontalHeaderLabels;
  horizontalHeaderLabels.append(  tr( "Tbtime" ) );
  horizontalHeaderLabels.append(  tr( "Observation" ) );
  horizontalHeaderLabels.append(  tr( "Message" ) );
  setHorizontalHeaderLabels(horizontalHeaderLabels);

  for ( int iRow = 0; iRow < rejList.size(); iRow++ ) {
    cout << rejList[iRow].tbtime() << " " << rejList[iRow].message() << endl;
    QTableWidgetItem* tbtimeItem = new QTableWidgetItem(rejList[iRow].tbtime().format("%e/%m %Y %H:%M:%S").cStr());
    setItem(iRow, 0, tbtimeItem);
    resizeColumnToContents(0);
    QString msg = rejList[iRow].message().cStr();
    static QRegExp regexp( ".*\\<data\\>(.*)\\<\\/data\\>.*", false );
    if ( regexp.exactMatch( msg ) ) {
      msg = regexp.cap(1);
    }
    QTableWidgetItem* messageItem = new QTableWidgetItem(msg);
    setItem(iRow, 1, messageItem);
    resizeColumnToContents(1);
    QTableWidgetItem* commentItem = new QTableWidgetItem(rejList[iRow].comment().cStr());
    setItem(iRow, 2, commentItem);
    resizeColumnToContents(2);
  }
  adjustSize();
}

Rejects::Rejects(vector<kvalobs::kvRejectdecode> rejList) {
  setGeometry(0,0,1200,1200);
  rTab = new RejectTable(rejList, this);
}
