
#ifndef AutoDataList_hh
#define AutoDataList_hh 1

#include "DynamicDataList.hh"
#include "common/ObsColumn.hh"

class QPushButton;
class QDomElement;

// ------------------------------------------------------------------------

class AutoDataList : public DynamicDataList
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
  typedef std::vector<Column> Columns_t;

protected:
  DataListModel* makeModel();
  std::string viewType() const;
  void changes(QDomElement& doc_changes);
  void replay(const QDomElement& doc_changes);
  void retranslateUi();

private Q_SLOTS:
  void onHorizontalHeaderContextMenu(const QPoint& pos);
  void onHorizontalHeaderSectionMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex);
  void onActionAddColumn();
  void onActionRemoveColumn();
  void onActionResetColumns();
  void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:
  void addColumnBefore(int column);
  void removeColumns(std::vector<int> columns);
  ObsColumn_p makeColumn(const Column& c);
  void generateColumns();

private:
  QMenu* mColumnMenu;
  QAction* mColumnAdd;
  QAction* mColumnRemove;
  QAction* mColumnReset;
  QPushButton* mButtonColumns;

  Columns_t mColumns,    mOriginalColumns;
};

#endif // AutoDataList_hh
