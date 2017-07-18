
#ifndef QSmallTable_HH
#define QSmallTable_HH

#include <QTableView>

class QSmallTable : public QTableView {
public:
  QSmallTable(QWidget* parent=0)
    : QTableView(parent) { }
  QSize sizeHint() const
    { return QSize(128, 48); }
};

#endif // QSmallTable_HH
