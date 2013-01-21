// based on http://doc.qt.digia.com/main-snapshot/itemviews-frozencolumn.html

#ifndef FREEZETABLEWIDGET_H
#define FREEZETABLEWIDGET_H

#include <QTableView>

class FreezeTableView : public QTableView {
    Q_OBJECT;
    
public:
    FreezeTableView(QWidget* parent=0);
    ~FreezeTableView();

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
    void updateFrozenTableGeometry();

private:
    QTableView *frozenTableView;
};
#endif
