
#include "CurrentTableView.hh"

void DataListTable::currentChanged(const QModelIndex& c, const QModelIndex& p)
{
  QTableView::currentChanged(c, p);
  //if (c.isValid())
    Q_EMIT signalCurrentChanged(c);
}
