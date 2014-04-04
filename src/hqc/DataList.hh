
#ifndef DataList_hh
#define DataList_hh 1

#include "access/EditAccess.hh"
#include "common/ModelAccess.hh"
#include "common/Sensor.hh"
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

class DataList : public QWidget
{ Q_OBJECT
public:
  DataList(QWidget* parent=0);
  ~DataList() = 0;

public Q_SLOTS:  
  virtual void navigateTo(const SensorTime&);
  
Q_SIGNALS:
  void signalNavigateTo(const SensorTime&);
  
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
  virtual void changeEvent(QEvent *event);
  const SensorTime& currentSensorTime() const
    { return mSensorTime; }
  virtual void doNavigateTo();

protected:
  std::auto_ptr<Ui::DataList> ui;
  std::auto_ptr<DataListModel> mTableModel;
  int mBlockNavigateTo;

private:
  EditAccess_p mDA;
  ModelAccessPtr mMA;
  SensorTime mSensorTime;
};

#endif // DataList_hh
