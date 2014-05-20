
#ifndef DataList_hh
#define DataList_hh 1

#include "AbstractDataView.hh"
#include "common/EditAccess.hh"
#include "common/ModelAccess.hh"
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
    { QTableView::currentChanged(c, p); if (c.isValid()) /* emit */ signalCurrentChanged(c); }
Q_SIGNALS:
  void signalCurrentChanged(const QModelIndex& c);
};

// ------------------------------------------------------------------------

class DataList : public AbstractDataView
{ Q_OBJECT
public:
  DataList(QWidget* parent=0);
  ~DataList() = 0;
                 
private Q_SLOTS:
  void onButtonSaveAs();
  void onCheckFilter(bool filterByTimestep);
  virtual void onCurrentChanged(const QModelIndex& current);
  virtual void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
  void onUITimeStepChanged(int index);
  void onModelTimeStepChanged(int step);
  void onModelFilterByTimeStepChanged(bool enabled, bool ftbs);

protected:
  void updateModel(DataListModel* model);
  virtual void doNavigateTo();
  virtual void retranslateUi();

private:
  void addTimeStepItem(int step);
  QString labelForStep(int step);

protected:
  std::auto_ptr<Ui::DataList> ui;
  std::auto_ptr<DataListModel> mTableModel;
  EditAccess_p mDA;
  ModelAccess_p mMA;
};

#endif // DataList_hh
