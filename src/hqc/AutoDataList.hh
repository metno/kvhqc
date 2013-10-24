
#ifndef AutoDataList_hh
#define AutoDataList_hh 1

#include "DataList.hh"
#include "common/ObsColumn.hh"

class QPushButton;
class QDomElement;

// ------------------------------------------------------------------------

class AutoDataList : public DataList
{ Q_OBJECT
public:
  AutoDataList(QWidget* parent=0);
  ~AutoDataList();
  
  virtual void navigateTo(const SensorTime&);
  
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
  struct eq_Column;
  struct lt_Column;
  typedef std::vector<Column> Columns_t;

protected:
  virtual void showEvent(QShowEvent* showEvent);
  virtual void hideEvent(QHideEvent* hideEvent);
  virtual void changeEvent(QEvent *event);

private Q_SLOTS:
  void onEarlier();
  void onLater();
  void onHorizontalHeaderContextMenu(const QPoint& pos);
  void onHorizontalHeaderSectionMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex);
  void onActionAddColumn();
  void onActionRemoveColumn();
  void onActionResetColumns();
  void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:
  void doNavigateTo(const SensorTime& st);
  void addColumnBefore(int column);
  void removeColumns(std::vector<int> columns);
  void makeModel();
  ObsColumnPtr makeColumn(const Column& c);
  std::string changes();
  void replay(const std::string& changes);
  void storeChanges();
  void generateColumns();

private:
  SensorTime mSensorTime;

  QMenu* mColumnMenu;
  QAction* mColumnAdd;
  QAction* mColumnRemove;
  QAction* mColumnReset;
  QPushButton* mButtonColumns;
  QPushButton* mButtonEarlier;
  QPushButton* mButtonLater;

  TimeRange mTimeLimits, mOriginalTimeLimits;
  Columns_t mColumns,    mOriginalColumns;

  bool mVisible;
  SensorTime mPendingSensorTime;
};

#endif // AutoDataList_hh
