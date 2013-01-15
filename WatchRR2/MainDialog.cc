
#include "MainDialog.hh"

#include "AnalyseFCC.hh"
#include "AnalyseRR24.hh"
#include "EditDialog.hh"
#include "Helpers.hh"
#include "KvStationBuffer.hh"
#include "MainTableModel.hh"
#include "NeighborDataModel.hh"
#include "NeighborTableModel.hh"
#include "ObsDelegate.hh"
#include "RedistDialog.hh"

#include "ui_watchrr_main.h"
#include "ui_watchrr_redist.h"

#include <kvalobs/kvDataOperations.h>
#include <QtGui/QMessageBox>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#define NDEBUG
#include "debug.hh"

using namespace Helpers;

MainDialog::MainDialog(EditAccessPtr da, ModelAccessPtr ma, const Sensor& sensor, const TimeRange& time)
    : ui(new Ui::DialogMain)
    , mDA(da)
    , mSensor(sensor)
    , mTime(time)
    , mRRModel(new MainTableModel(mDA, ma, mSensor, mTime))
    , mNeighborModel(new NeighborTableModel(mDA, mSensor, mTime))
    , mNeighborData(new NeighborDataModel(mDA, mSensor))
{
    ui->setupUi(this);

    initializeRR24Data();

    QString info = tr("Station %1 [%2]").arg(mSensor.stationId).arg(mSensor.typeId);
    try {
        const kvalobs::kvStation& s = KvStationBuffer::instance()->findStation(mSensor.stationId);
        info += " " + QString::fromStdString(s.name());
        if (s.environmentid() == 10)
            info += " " + tr("[not daily]");
    } catch(...) {
        // TODO handle errors
    }
    ui->labelStationInfo->setText(info);

    QFont mono("Monospace");

    ui->buttonSave->setEnabled(false);
    ui->tableRR->setModel(mRRModel.get());
    ui->tableRR->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
    ui->tableRR->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    ui->tableRR->setItemDelegate(new ObsDelegate(this));
    ui->tableRR->verticalHeader()->setFont(mono);
    ui->labelInfo->setText("");
    ui->buttonUndo->setEnabled(false);
    ui->buttonRedo->setVisible(false);

    ui->tableNeighborRR->setModel(mNeighborModel.get());
    ui->tableNeighborRR->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
    ui->tableNeighborRR->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    ui->tableNeighborRR->verticalHeader()->setResizeMode(QHeaderView::Interactive);
    ui->tableNeighborRR->verticalHeader()->resizeSections(QHeaderView::ResizeToContents);
    ui->tableNeighborRR->verticalHeader()->setFont(mono);

    ui->tableNeighborData->setModel(mNeighborData.get());
    ui->tableNeighborData->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
    ui->tableNeighborData->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    ui->tableNeighborData->verticalHeader()->setResizeMode(QHeaderView::Interactive);
    ui->tableNeighborData->verticalHeader()->resizeSections(QHeaderView::ResizeToContents);

    const boost::gregorian::date d0 = time.t0().date(), d1 = time.t1().date();
    ui->dateNeighborData->setMinimumDate(QDate(d0.year(), d0.month(), d0.day()));
    ui->dateNeighborData->setMaximumDate(QDate(d1.year(), d1.month(), d1.day()));
    ui->dateNeighborData->setDate(QDate(d0.year(), d0.month(), d0.day()+1));

    connect(ui->tableRR->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this, SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));
    connect(mRRModel.get(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(onDataChanged(const QModelIndex&,const QModelIndex&)));

    mDA->backendDataChanged.connect(boost::bind(&MainDialog::onBackendDataChanged, this, _1, _2));
}

MainDialog::~MainDialog()
{
    mDA->backendDataChanged.disconnect(boost::bind(&MainDialog::onBackendDataChanged, this, _1, _2));
}

void MainDialog::initializeRR24Data()
{
    mEditableTime = mTime;
    RR24::analyse(mDA, mSensor, mEditableTime);
    FCC::analyse(mDA, mSensor, mEditableTime);
    mRRModel->setRR24TimeRange(mEditableTime);
    mDA->pushUpdate();
}

void MainDialog::onSelectionChanged(const QItemSelection&, const QItemSelection&)
{
    const Selection sel = findSelection();
    if( sel.empty() ) {
        ui->labelInfo->setText("");
        ui->buttonEdit->setEnabled(false);
        ui->buttonRedist->setEnabled(false);
        ui->buttonRedistQC2->setEnabled(false);
        ui->buttonAcceptRow->setEnabled(false);
        return;
    }

    const int nDays = sel.selTime.days() + 1;

    if( sel.minCol == mRRModel->getRR24Column() and sel.minCol == sel.maxCol ) {
        ui->buttonEdit->setEnabled(true);
        ui->buttonAcceptRow->setEnabled(false);
        if( nDays <= 1 ) {
            ui->labelInfo->setText("");
            ui->buttonRedist->setEnabled(false);
        } else {
            const float sum = RR24::calculateSum(mDA, mSensor, sel.selTime);
            ui->labelInfo->setText(tr("Sum: %1mm").arg(QString::number(sum, 'f', 1)));
            ui->buttonRedist->setEnabled(true);
        }
        ui->buttonRedistQC2->setEnabled(RR24::canRedistributeInQC2(mDA, mSensor, sel.selTime));
    } else {
        ui->buttonEdit->setEnabled(false);
        ui->buttonRedist->setEnabled(false);
        ui->buttonRedistQC2->setEnabled(false);

        const bool completeSingleRow = (sel.selTime.t0() == sel.selTime.t1())
            and (sel.minCol == 0 and sel.maxCol >= mRRModel->columnCount(QModelIndex()) - 2);
        ui->buttonAcceptRow->setEnabled(completeSingleRow);
    }
}

MainDialog::Selection MainDialog::findSelection()
{
    QModelIndexList selected = ui->tableRR->selectionModel()->selectedIndexes();
    if( selected.isEmpty() )
        return Selection();

    // selected is not sorted (Qt docs), need to find range
    Selection s;
    int minRow = -1, maxRow = -1;
    for (int i=0; i<selected.count(); i++) {
        const int r = selected.at(i).row(), c = selected.at(i).column();
        if( minRow == -1 or minRow > r )
            minRow = r;
        if( maxRow < r )
            maxRow = r;
        if( s.minCol == -1 or s.minCol > c )
            s.minCol = c;
        if( s.maxCol < c )
            s.maxCol = c;
    }
    s.selTime = TimeRange(mRRModel->timeAtRow(minRow), mRRModel->timeAtRow(maxRow));
    return s;
}

void MainDialog::onAcceptRow()
{
    const Selection sel = findSelection();
    FCC::acceptRow(mDA, mSensor, sel.selTime.t0());
    ui->tableRR->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    enableSave();
    clearSelection();
}

void MainDialog::reject()
{
    int updates = mDA->countU();
    if (updates > 0) {
        QMessageBox w(this);
        w.setWindowTitle(windowTitle());
        w.setIcon(QMessageBox::Warning);
        w.setText(tr("There are %1 unsaved data updates.").arg(updates));
        w.setInformativeText(tr("Are you sure that you want to lose them?"));
        QPushButton* discard = w.addButton(tr("Discard changes"), QMessageBox::ApplyRole);
        QPushButton* cont = w.addButton(tr("Continue"), QMessageBox::RejectRole);
        w.setDefaultButton(cont);
        w.exec();
        if (w.clickedButton() != discard)
            return;
    }
    QDialog::reject();
}

void MainDialog::onEdit()
{
    const Selection sel = findSelection();
    EditAccessPtr eda = boost::make_shared<EditAccess>(mDA);
    EditDialog edit(this, eda, mSensor, sel.selTime, mEditableTime);
    if (edit.exec()) {
        mDA->pushUpdate();
        eda->sendChangesToParent();
        ui->tableRR->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
        enableSave();
        clearSelection();
    }
}

void MainDialog::onRedistribute()
{
    const Selection sel = findSelection();
    EditAccessPtr eda = boost::make_shared<EditAccess>(mDA);
    RedistDialog redist(this, eda, mSensor, sel.selTime, mEditableTime);
    if (redist.exec()) {
        mDA->pushUpdate();
        eda->sendChangesToParent();
        ui->tableRR->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
        enableSave();
        clearSelection();
    }
}

void MainDialog::onRedistributeQC2()
{
    const Selection sel = findSelection();
    RR24::redistributeInQC2(mDA, mSensor, sel.selTime, mEditableTime);
    ui->tableRR->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    enableSave();
    clearSelection();
}

void MainDialog::onUndo()
{
    if (mDA->countUpdates() > 0) {
        mDA->popUpdate();
        enableSave();
        clearSelection();
    }
}

void MainDialog::clearSelection()
{
    const QModelIndex tl = mRRModel->index(0, 0, QModelIndex());
    const QModelIndex br = mRRModel->index(mRRModel->rowCount(QModelIndex())-1,
                                           mRRModel->columnCount(QModelIndex())-1, QModelIndex());
    QItemSelection s;
    s.select(tl, br);
    ui->tableRR->selectionModel()->select(s, QItemSelectionModel::Clear);
}

void MainDialog::enableSave()
{
    LOG_SCOPE();
    int updates = mDA->countU(), tasks = mDA->countT();

    ui->buttonSave->setEnabled(tasks == 0 and updates > 0);
    ui->buttonUndo->setEnabled((mDA->countUpdates() > 1) and (updates > 0));
}

void MainDialog::onDataChanged(const QModelIndex&, const QModelIndex&)
{
    enableSave();
}

void MainDialog::onBackendDataChanged(ObsAccess::ObsDataChange what, EditDataPtr ebs)
{
    if (ebs->modified() or ebs->hasTasks()) {
        QMessageBox w(this);
        w.setWindowTitle(windowTitle());
        w.setIcon(QMessageBox::Warning);
        w.setText(tr("Kvalobs data you are editing have been changed. You will have to start over again. Sorry!"));
        QPushButton* discard = w.addButton(tr("Quit and Discard changes"), QMessageBox::ApplyRole);
        w.setDefaultButton(discard);
        w.exec();
        QDialog::reject();
    }
}

void MainDialog::onNeighborDataDateChanged(const QDate& date)
{
    QDateTime qdt(date, QTime(mTime.t0().time_of_day().hours(), 0, 0));
    mNeighborData->setTime(timeutil::from_QDateTime(qdt));
    ui->tableNeighborData->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}
