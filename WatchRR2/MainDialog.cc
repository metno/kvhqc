
#include "MainDialog.hh"

#include "AnalyseFCC.hh"
#include "AnalyseRR24.hh"
#include "EditDialog.hh"
#include "Helpers.hh"
#include "MainTableModel.hh"
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
    , mModel(new MainTableModel(mDA, ma, mSensor, mTime))
{
    ui->setupUi(this);

    initializeRR24Data();

    ui->labelStationInfo->setText(tr("Station %1 [%2]").arg(mSensor.stationId).arg(mSensor.typeId));
    ui->buttonSave->setEnabled(false);
    ui->rrTable->setModel(mModel.get());
    ui->rrTable->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
    ui->rrTable->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    ui->rrTable->setItemDelegate(new ObsDelegate(this));
    ui->labelInfo->setText("");
    ui->buttonUndo->setEnabled(false);
    ui->buttonRedo->setVisible(false);

    connect(ui->rrTable->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this, SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));
    connect(mModel.get(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
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
    mModel->setRR24TimeRange(mEditableTime);
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

    if( sel.minCol == mModel->getRR24Column() and sel.minCol == sel.maxCol ) {
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
            and (sel.minCol == 0 and sel.maxCol >= mModel->columnCount(QModelIndex()) - 2);
        ui->buttonAcceptRow->setEnabled(completeSingleRow);
    }
}

MainDialog::Selection MainDialog::findSelection()
{
    QModelIndexList selected = ui->rrTable->selectionModel()->selectedIndexes();
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
    s.selTime = TimeRange(mModel->timeAtRow(minRow), mModel->timeAtRow(maxRow));
    return s;
}

void MainDialog::onAcceptRow()
{
    const Selection sel = findSelection();
    FCC::acceptRow(mDA, mSensor, sel.selTime.t0());
    ui->rrTable->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
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
        ui->rrTable->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
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
        ui->rrTable->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
        enableSave();
        clearSelection();
    }
}

void MainDialog::onRedistributeQC2()
{
    const Selection sel = findSelection();
    RR24::redistributeInQC2(mDA, mSensor, sel.selTime, mEditableTime);
    ui->rrTable->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
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
    const QModelIndex tl = mModel->index(0, 0, QModelIndex());
    const QModelIndex br = mModel->index(mModel->rowCount(QModelIndex())-1,
                                         mModel->columnCount(QModelIndex())-1, QModelIndex());
    QItemSelection s;
    s.select(tl, br);
    ui->rrTable->selectionModel()->select(s, QItemSelectionModel::Clear);
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
