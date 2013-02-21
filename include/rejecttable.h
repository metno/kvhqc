// -*- c++ -*-

#ifndef REJECTTABLE_H
#define REJECTTABLE_H

#include <QtGui/QDialog>
#include <QtGui/QTableWidget>
#include <vector>

namespace kvalobs {
class kvRejectdecode;
}

class RejectTable : public QTableWidget
{ Q_OBJECT;
public:
    RejectTable(const std::vector<kvalobs::kvRejectdecode>&, QWidget* parent);
};

class Rejects : public QDialog
{ Q_OBJECT;
public:
    Rejects(const std::vector<kvalobs::kvRejectdecode>&, QWidget* parent=0);
    RejectTable* rTab;
};

#endif
