// based on http://doc.qt.digia.com/main-snapshot/itemviews-frozencolumn.html

#include <QtGui>

#include "FrozenRowTableView.hh"

FrozenRowTableView::FrozenRowTableView(QWidget* parent)
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

    connect(frozenTableView->horizontalScrollBar(), SIGNAL(valueChanged(int)),
            horizontalScrollBar(), SLOT(setValue(int)));
    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)),
            frozenTableView->horizontalScrollBar(), SLOT(setValue(int)));
}

FrozenRowTableView::~FrozenRowTableView()
{
    delete frozenTableView;
}

void FrozenRowTableView::setModel(QAbstractItemModel* mdl)
{
    QTableView::setModel(mdl);
    frozenTableView->setModel(model());
    frozenTableView->setSelectionModel(selectionModel());
    initFrozenTableGeometry();
}

void FrozenRowTableView::initFrozenTableGeometry()
{
    if (model()) {
        const int rc = model()->rowCount();
        for(int row=1; row<rc; row++)
            frozenTableView->setRowHidden(row, true);
        const int rh = (rc>0) ? rowHeight(0) : 0;
        frozenTableView->setRowHeight(0, rh);
        updateFrozenTableGeometry();
    }
}

void FrozenRowTableView::init()
{
    frozenTableView->setFocusPolicy(Qt::NoFocus);
    frozenTableView->horizontalHeader()->hide();
    frozenTableView->verticalHeader()->setResizeMode(QHeaderView::Fixed);

    viewport()->stackUnder(frozenTableView);

    frozenTableView->setStyleSheet("QTableView { border: none; }");
    frozenTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    frozenTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    frozenTableView->show();

    setHorizontalScrollMode(ScrollPerPixel);
    setVerticalScrollMode(ScrollPerPixel);
    frozenTableView->setHorizontalScrollMode(ScrollPerPixel);

    initFrozenTableGeometry();
}

void FrozenRowTableView::updateSectionWidth(int logicalIndex, int, int newSize)
{
    frozenTableView->setColumnWidth(logicalIndex, newSize);
}

void FrozenRowTableView::updateSectionHeight(int logicalIndex, int, int newSize)
{
    if (logicalIndex == 0) {
        frozenTableView->setRowHeight(0, newSize);
        updateFrozenTableGeometry();
    }
}

void FrozenRowTableView::resizeEvent(QResizeEvent * event)
{
    QTableView::resizeEvent(event);
    updateFrozenTableGeometry();
}

QModelIndex FrozenRowTableView::moveCursor(CursorAction cursorAction,
                                           Qt::KeyboardModifiers modifiers)
{
    QModelIndex current = QTableView::moveCursor(cursorAction, modifiers);

    const int vy = visualRect(current).topLeft().y(), frh = frozenTableView->rowHeight(0);
    if(cursorAction == MoveUp && current.row()>0 and vy < frh) {
        const int newValue = verticalScrollBar()->value() + vy - frh;
        verticalScrollBar()->setValue(newValue);
    }
    return current;
}

void FrozenRowTableView::scrollTo (const QModelIndex & index, ScrollHint hint)
{
    if (index.row() > 0)
        QTableView::scrollTo(index, hint);
}

void FrozenRowTableView::updateFrozenTableGeometry()
{
    if (not model() or model()->rowCount() == 0)
        return;

    QHeaderView* fvh = frozenTableView->verticalHeader();
    const int fw = frameWidth(), vw = verticalHeader()->width();
    fvh->setFixedWidth(vw);
    frozenTableView->setGeometry(fw, horizontalHeader()->height()+fw,
                                 viewport()->width()+vw, rowHeight(0));
}
