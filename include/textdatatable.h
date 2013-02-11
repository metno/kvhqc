// -*- c++ -*-

#ifndef TEXTDATATABLE_H
#define TEXTDATATABLE_H

#include <QtGui/QTableWidget>
#include "hqcdefs.h"

class TextDataTable : public QTableWidget
{
 public:
  TextDataTable(const std::vector<TxtDat>&, QWidget*);
};

class TextData : public QWidget {
Q_OBJECT
  public:
  TextData(const std::vector<TxtDat>&);
  TextDataTable* txtTab;
};

#endif
