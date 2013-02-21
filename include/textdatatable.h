// -*- c++ -*-

#ifndef TEXTDATATABLE_H
#define TEXTDATATABLE_H

#include "hqcdefs.h"

#include <QtGui/QDialog>
#include <QtGui/QTableWidget>

class TextDataTable : public QTableWidget
{ Q_OBJECT;
public:
    TextDataTable(const std::vector<TxtDat>&, QWidget*);
};

class TextData : public QDialog
{ Q_OBJECT;
public:
    TextData(const std::vector<TxtDat>&, QWidget* parent=0);
    TextDataTable* txtTab;
};

#endif
