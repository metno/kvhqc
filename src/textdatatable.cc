#include "textdatatable.h"
#include <iostream>
#include <QSizePolicy>

using namespace std;

TextDataTable::TextDataTable(vector<TxtDat> txtList, QMap<int,QString> parMap, QWidget* parent)
  : QTableWidget(3000,6,parent) {

  setWindowTitle("TextData");
  setRowCount(txtList.size());
  setGeometry(10,10,700,1200);
  setMinimumWidth(620);
  setMinimumHeight(1000);
  QFont font("Courier", 9, QFont::DemiBold);
  setFont(font);  
  QStringList horizontalHeaderLabels;
  horizontalHeaderLabels.append(  tr( "Stationid" ) );
  horizontalHeaderLabels.append(  tr( "Obstime" ) );
  horizontalHeaderLabels.append(  tr( "Original " ) );
  horizontalHeaderLabels.append(  tr( "Paramid " ) );
  horizontalHeaderLabels.append(  tr( "ParamName ") );
  horizontalHeaderLabels.append(  tr( "Tbtime " ) );
  horizontalHeaderLabels.append(  tr( "Typeid " ) );
  setHorizontalHeaderLabels(horizontalHeaderLabels);

  for ( int iRow = 0; iRow < txtList.size(); iRow++ ) {
    QTableWidgetItem* stationItem = new QTableWidgetItem(QString::number(txtList[iRow].stationId));
    setItem(iRow, 0, stationItem);
    resizeColumnToContents(0);
    //    QTableWidgetItem* obstimeItem = new QTableWidgetItem(txtList[iRow].obstime.format("%e/%m %Y %H:%M:%S").cStr());
    QTableWidgetItem* obstimeItem = new QTableWidgetItem(txtList[iRow].obstime.format("%Y-%m-%e %H:%M:%S").cStr());
    setItem(iRow, 1, obstimeItem);
    resizeColumnToContents(1);
    QTableWidgetItem* originalItem = new QTableWidgetItem(QString(txtList[iRow].original.c_str()));
    setItem(iRow, 2, originalItem);
    resizeColumnToContents(2);
    QTableWidgetItem* paramItem = new QTableWidgetItem(QString::number(txtList[iRow].paramId));
    setItem(iRow, 3, paramItem);
    resizeColumnToContents(3);
    QTableWidgetItem* paramNameItem = new QTableWidgetItem(QString(parMap[txtList[iRow].paramId]));
    setItem(iRow, 4, paramNameItem);
    resizeColumnToContents(4);
    QTableWidgetItem* tbtimeItem = new QTableWidgetItem(txtList[iRow].tbtime.format("%Y-%m-%e %H:%M:%S").cStr());
    setItem(iRow, 5, tbtimeItem);
    resizeColumnToContents(5);
    QTableWidgetItem* typeItem = new QTableWidgetItem(QString::number(txtList[iRow].typeId));
    setItem(iRow, 6, typeItem);
    resizeColumnToContents(6);
  }
  adjustSize();
}

TextData::TextData(vector<TxtDat> txtList, QMap<int,QString> parMap) {
  setGeometry(0,0,700,1200);
  txtTab = new TextDataTable(txtList, parMap, this);
}
