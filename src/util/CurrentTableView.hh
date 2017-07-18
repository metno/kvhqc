
#ifndef CurrentTableView_hh
#define CurrentTableView_hh 1

#include <QTableView>

class DataListTable : public QTableView
{ Q_OBJECT
public:
  DataListTable(QWidget* parent=0)
    : QTableView(parent) { }

  void currentChanged(const QModelIndex& c, const QModelIndex& p);

Q_SIGNALS:
  void signalCurrentChanged(const QModelIndex& c);
};

#endif // CurrentTableView_hh
