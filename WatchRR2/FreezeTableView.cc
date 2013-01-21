// based on http://doc.qt.digia.com/main-snapshot/itemviews-frozencolumn.html

#include <QtGui>

#include "FreezeTableView.hh"

FreezeTableView::FreezeTableView(QWidget* parent)
    : QTableView(parent)
    , frozenTableView(new QTableView(this))
{
    init();

    //connect the headers and scrollbars of both tableviews together
    connect(horizontalHeader(),SIGNAL(sectionResized(int,int,int)),
            this, SLOT(updateSectionWidth(int,int,int)));
    connect(verticalHeader(),SIGNAL(sectionResized(int,int,int)),
            this, SLOT(updateSectionHeight(int,int,int)));
    connect(horizontalHeader(),SIGNAL(geometriesChanged()),
            this, SLOT(updateFrozenTableGeometry()));
    connect(verticalHeader(),SIGNAL(geometriesChanged()),
            this, SLOT(updateFrozenTableGeometry()));

    connect(frozenTableView->verticalScrollBar(), SIGNAL(valueChanged(int)),
            verticalScrollBar(), SLOT(setValue(int)));
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)),
            frozenTableView->verticalScrollBar(), SLOT(setValue(int)));
}

FreezeTableView::~FreezeTableView()
{
    delete frozenTableView;
}

void FreezeTableView::setModel(QAbstractItemModel* mdl)
{
    QTableView::setModel(mdl);
    frozenTableView->setModel(model());
    frozenTableView->setSelectionModel(selectionModel());
    initFrozenTableGeometry();
}

void FreezeTableView::initFrozenTableGeometry()
{
    if (model()) {
        const int cc = model()->columnCount();
        for(int col=1; col<cc; col++)
            frozenTableView->setColumnHidden(col, true);
        const int cw = (cc>0) ? columnWidth(0) : 0;
        frozenTableView->setColumnWidth(0, cw);
        updateFrozenTableGeometry();
    }
}

void FreezeTableView::init()
{
    frozenTableView->setFocusPolicy(Qt::NoFocus);
    frozenTableView->verticalHeader()->hide();
    frozenTableView->horizontalHeader()->setResizeMode(QHeaderView::Fixed);

    viewport()->stackUnder(frozenTableView);

    frozenTableView->setStyleSheet("QTableView { border: none; }");
    frozenTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    frozenTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    frozenTableView->show();

    setHorizontalScrollMode(ScrollPerPixel);
    setVerticalScrollMode(ScrollPerPixel);
    frozenTableView->setVerticalScrollMode(ScrollPerPixel);

    initFrozenTableGeometry();
}

void FreezeTableView::updateSectionWidth(int logicalIndex, int, int newSize)
{
    if (logicalIndex == 0) {
        frozenTableView->setColumnWidth(0,newSize);
        updateFrozenTableGeometry();
    }
}

void FreezeTableView::updateSectionHeight(int logicalIndex, int, int newSize)
{
    frozenTableView->setRowHeight(logicalIndex, newSize);
}

void FreezeTableView::resizeEvent(QResizeEvent * event)
{
    QTableView::resizeEvent(event);
    updateFrozenTableGeometry();
}

QModelIndex FreezeTableView::moveCursor(CursorAction cursorAction,
                                          Qt::KeyboardModifiers modifiers)
{
    QModelIndex current = QTableView::moveCursor(cursorAction, modifiers);

    if(cursorAction == MoveLeft && current.column()>0
       && visualRect(current).topLeft().x() < frozenTableView->columnWidth(0) ){

        const int newValue = horizontalScrollBar()->value() + visualRect(current).topLeft().x()
            - frozenTableView->columnWidth(0);
        horizontalScrollBar()->setValue(newValue);
    }
    return current;
}

void FreezeTableView::scrollTo (const QModelIndex & index, ScrollHint hint)
{
    if (index.column() > 0)
        QTableView::scrollTo(index, hint);
}

void FreezeTableView::updateFrozenTableGeometry()
{
    if (not model() or model()->columnCount() == 0)
        return;
    frozenTableView->setGeometry(verticalHeader()->width()+frameWidth(),
                                 frameWidth(), columnWidth(0),
                                 viewport()->height()+horizontalHeader()->height());
}
