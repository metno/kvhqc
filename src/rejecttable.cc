
#include "rejecttable.h"
#include "timeutil.hh"
#include <kvalobs/kvRejectdecode.h>
#include <QtGui/QVBoxLayout>
#include <iostream>

using namespace std;

RejectTable::RejectTable(const std::vector<kvalobs::kvRejectdecode>& rejList, QWidget* parent)
  : QTableWidget(3000,3,parent) {

  setWindowTitle(tr("Rejected"));
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

  for ( unsigned int iRow = 0; iRow < rejList.size(); iRow++ ) {
    cout << timeutil::to_iso_extended_string(timeutil::from_miTime(rejList[iRow].tbtime()))
         << " " << rejList[iRow].message() << endl;
    QTableWidgetItem* tbtimeItem = new QTableWidgetItem(QString::fromStdString(timeutil::to_iso_extended_string(timeutil::from_miTime(rejList[iRow].tbtime()))));
    // original format: format("%e/%m %Y %H:%M:%S")
    setItem(iRow, 0, tbtimeItem);
    resizeColumnToContents(0);
    QString msg = QString::fromStdString(rejList[iRow].message());
    static QRegExp regexp( ".*\\<data\\>(.*)\\<\\/data\\>.*", false );
    if ( regexp.exactMatch( msg ) ) {
      msg = regexp.cap(1);
    }
    QTableWidgetItem* messageItem = new QTableWidgetItem(msg);
    setItem(iRow, 1, messageItem);
    resizeColumnToContents(1);
    QTableWidgetItem* commentItem = new QTableWidgetItem(QString::fromStdString(rejList[iRow].comment()));
    setItem(iRow, 2, commentItem);
    resizeColumnToContents(2);
  }
  adjustSize();
}

Rejects::Rejects(const std::vector<kvalobs::kvRejectdecode>& rejList, QWidget* parent)
    : QDialog(parent)
{
    setCaption(tr("Rejected"));
    resize(1200, 1000);

    rTab = new RejectTable(rejList, this);
    QVBoxLayout* topLayout = new QVBoxLayout(this);
    topLayout->addWidget(rTab);

    show();
}
