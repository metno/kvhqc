
#ifndef DataList_hh
#define DataList_hh 1

#include "ChangeableDataView.hh"
#include "ObsColumn.hh"

#include <QtGui/QTableView>

class QDomElement;
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

class DataList : public QWidget, public ChangeableDataView
{ Q_OBJECT
public:
  DataList(QWidget* parent=0);
  ~DataList();
  
  virtual void setSensorsAndTimes(const Sensors_t& sensors, const TimeRange& limits);
  
  virtual void navigateTo(const SensorTime&);
  
  virtual std::string changes();
  virtual void replay(const std::string& changes);
  virtual std::string type() const;
  virtual std::string id() const;

  void setShowDistances(bool showDistances)
    { mShowDistances = showDistances; }
  
public:
  enum ColumnType { CORRECTED, ORIGINAL, FLAGS, MODEL };
  
private:
  struct Column {
    Sensor sensor;
    ColumnType type;
    int timeOffset;
    Column() : type(CORRECTED), timeOffset(0) { }
    void toText(QDomElement& ce) const;
    void fromText(const QDomElement& ce);
  };
  struct lt_Column;
  typedef std::vector<Column> Columns_t;

private Q_SLOTS:
  void onEarlier();
  void onLater();
  void onHorizontalHeaderContextMenu(const QPoint& pos);
  void onHorizontalHeaderSectionMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex);
  void onButtonSaveAs();
  void onActionAddColumn();
  void onActionRemoveColumn();
  void onActionResetColumns();
  void currentChanged(const QModelIndex& current);
  void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:
  void addColumnBefore(int column);
  void removeColumns(std::vector<int> columns);
  void updateModel();
  ObsColumnPtr makeColumn(const Column& c);

  friend class DataListTable;

private:
  std::auto_ptr<Ui::DataList> ui;
  std::auto_ptr<DataListModel> mTableModel;
  SensorTime mSensorTime;

  QMenu* mColumnMenu;
  QAction* mColumnAdd;
  QAction* mColumnRemove;
  QAction* mColumnReset;

  TimeRange mTimeLimits, mOriginalTimeLimits;
  Columns_t mColumns,    mOriginalColumns;

  bool mShowDistances;
};

#endif // DataList_hh
