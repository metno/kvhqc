#ifndef REJECTTABLE_H
#define REJECTTABLE_H

#include <QtGui/QTableWidget>
#include <kvalobs/kvRejectdecode.h>
#include <vector>

class RejectTable : public QTableWidget
{
 public:
  RejectTable(std::vector<kvalobs::kvRejectdecode>, QWidget*);
};

class Rejects : public QWidget {
Q_OBJECT
  public:
  Rejects(std::vector<kvalobs::kvRejectdecode>);
  RejectTable* rTab;
};

#endif
