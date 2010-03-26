#ifndef REJECTTABLE_H
#define REJECTTABLE_H

#include <QTableWidget>
#include <kvskel/kvService.hh>
#include <kvcpp/KvApp.h>
using namespace std;

class RejectTable : public QTableWidget
{
 public:
  RejectTable(vector<kvalobs::kvRejectdecode>, QWidget*);
};

class Rejects : public QWidget {
Q_OBJECT
  public:
  Rejects(vector<kvalobs::kvRejectdecode>);
  RejectTable* rTab;
};

#endif
