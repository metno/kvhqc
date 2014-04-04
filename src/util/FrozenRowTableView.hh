// based on http://doc.qt.digia.com/main-snapshot/itemviews-frozencolumn.html

#ifndef FROZENROWTABLEVIEW_H
#define FROZENROWTABLEVIEW_H

#include <QTableView>

class FrozenRowTableView : public QTableView 
{ Q_OBJECT;
  
public:
  FrozenRowTableView(QWidget* parent=0);
  ~FrozenRowTableView();

  virtual void setModel(QAbstractItemModel* model);
    
protected:
  virtual void resizeEvent(QResizeEvent *event);
  virtual QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);
  void scrollTo (const QModelIndex & index, ScrollHint hint = EnsureVisible);
    
private:
  void init();
  void initFrozenTableGeometry();

private Q_SLOTS:
  void updateSectionWidth(int logicalIndex,int, int newSize);
  void updateSectionHeight(int logicalIndex, int, int newSize);
  void frozenSectionResized(int logicalIndex,int, int newSize);
  void updateFrozenTableGeometry();

private:
  QTableView *frozenTableView;
};
#endif
