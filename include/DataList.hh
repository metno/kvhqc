
#ifndef DataList_hh
#define DataList_hh 1

#include "DataView.hh"

#include <QtGui/QTableView>

class DataListModel;
namespace Ui {
class DataList;
}

class DataListTable : public QTableView
{ Q_OBJECT
public:
  DataListTable(QWidget* parent=0) : QTableView(parent) { }
  void currentChanged(const QModelIndex& c, const QModelIndex& p)
    { QTableView::currentChanged(c, p); if (c.isValid()) /* emit */ currentChanged(c); }
Q_SIGNALS:
  void currentChanged(const QModelIndex& c);
};

// ------------------------------------------------------------------------

class DataList : public QWidget, public DataView
{ Q_OBJECT
public:
  DataList(QWidget* parent=0);
  ~DataList() = 0;
  
  virtual void navigateTo(const SensorTime&);
  
private Q_SLOTS:
  void onButtonSaveAs();
  virtual void currentChanged(const QModelIndex& current);
  virtual void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

protected:
  void updateModel(DataListModel* model);

protected:
  std::auto_ptr<Ui::DataList> ui;
  std::auto_ptr<DataListModel> mTableModel;
  SensorTime mSensorTime;
  bool mEmittingNavigateTo;
};

#endif // DataList_hh
