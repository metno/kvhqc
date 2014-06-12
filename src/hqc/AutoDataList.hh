
#ifndef AutoDataList_hh
#define AutoDataList_hh 1

#include "TimespanDataList.hh"

class ObsPgmRequest;
class QPushButton;

// ------------------------------------------------------------------------

class AutoDataList : public TimespanDataList
{ Q_OBJECT
public:
  AutoDataList(QWidget* parent=0);
  ~AutoDataList();

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
  typedef std::vector<Column> Column_v;

protected:
  void updateModel();
  void doSensorSwitch();

  std::string viewType() const;
  void switchSensorPrepare();
  void loadChangesXML(const QDomElement& doc_changes);
  void storeChangesXML(QDomElement& doc_changes);

  void retranslateUi();

private Q_SLOTS:
  void onHorizontalHeaderContextMenu(const QPoint& pos);
  void onHorizontalHeaderSectionMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex);
  void onActionAddColumn();
  void onActionRemoveColumn();
  void onActionResetColumns();
  void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

  void onObsPgmsComplete();

private:
  void addColumnBefore(int column);
  void removeColumns(std::vector<int> columns);
  ObsColumn_p makeColumn(const Column& c);
  bool hasChangedColumns() const;

private:
  QMenu* mColumnMenu;
  QAction* mColumnAdd;
  QAction* mColumnRemove;
  QAction* mColumnReset;
  QPushButton* mButtonColumns;

  Column_v mColumns, mOriginalColumns;
  ObsPgmRequest* mObsPgmRequest;
};

#endif // AutoDataList_hh
