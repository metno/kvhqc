#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>
#include <iostream>
#include <QString>
#include <QList>
#include "connect2stinfosys.h"

bool connect2stinfosys()
{
  QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
  db.setHostName("stinfosys");
  db.setDatabaseName("stinfosys");
  db.setPort(5435);
  db.setUserName("pstinfosys");
  db.setPassword("info12");
  if (!db.open()) {
    QMessageBox::critical(0, "Databaseproblem",
			  "Får ikke kontakt med stinfosys.\n\n"
			  "Klikk Cancel for å avslutte.", QMessageBox::Cancel);
    
    return false;
  }
  return true;
}
