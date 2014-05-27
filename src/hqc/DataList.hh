
#ifndef DataList_hh
#define DataList_hh 1

#include "DynamicDataView.hh"
#include "common/EditAccess.hh"
#include "common/ModelAccess.hh"
#include "util/BusyLabel.hh"
#include "util/CurrentTableView.hh"

class DataListModel;
class Ui_DataList;

class DataList : public DynamicDataView
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
  void onBusyStatus(bool);

protected:
  void doNavigateTo();
  void retranslateUi();
  DataListModel* model() const;

private:
  void addTimeStepItem(int step);
  QString labelForStep(int step);
  void selectCurrent();

protected:
  std::auto_ptr<Ui_DataList> ui;
  BusyLabel* mBusy;
  EditAccess_p mDA;
  ModelAccess_p mMA;
  bool mCurrentSelected;
};

#endif // DataList_hh
