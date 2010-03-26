#ifndef TEXTDATATABLE_H
#define TEXTDATATABLE_H

#include <QTableWidget>
#include <QMap>
#include <kvskel/kvService.hh>
#include <kvcpp/KvApp.h>
#include "hqcdefs.h"

using namespace std;

class TextDataTable : public QTableWidget
{
 public:
  TextDataTable(vector<TxtDat>, QMap<int,QString> parMap, QWidget*);
};

class TextData : public QWidget {
Q_OBJECT
  public:
  TextData(vector<TxtDat>, QMap<int,QString> parMap);
  TextDataTable* txtTab;
};

#endif
