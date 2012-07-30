#ifndef TEXTDATATABLE_H
#define TEXTDATATABLE_H

#include <QtGui/QTableWidget>
#include <QtCore/QMap>
#include "hqcdefs.h"

class TextDataTable : public QTableWidget
{
 public:
  TextDataTable(std::vector<TxtDat>, QMap<int,QString> parMap, QWidget*);
};

class TextData : public QWidget {
Q_OBJECT
  public:
  TextData(std::vector<TxtDat>, QMap<int,QString> parMap);
  TextDataTable* txtTab;
};

#endif
