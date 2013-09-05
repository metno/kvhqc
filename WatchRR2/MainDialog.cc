
#include "MainDialog.hh"

#include "AnalyseFCC.hh"
#include "AnalyseRR24.hh"
#include "BusyIndicator.h"
#include "DianaHelper.hh"
#include "EditDialog.hh"
#include "Helpers.hh"
#include "KvMetaDataBuffer.hh"
#include "MainTableModel.hh"
#include "NeighborDataModel.hh"
#include "NeighborTableModel.hh"
#include "ObsDelegate.hh"
#include "RedistDialog.hh"

#include "ui_watchrr_main.h"
#include "ui_watchrr_redist.h"

#include <kvalobs/kvDataOperations.h>
#ifdef METLIBS_BEFORE_4_9_5
#define signals Q_SIGNALS
#define slots Q_SLOTS
#endif
#include <qUtilities/ClientButton.h>
#include <QtCore/qsettings.h>
#include <QtGui/QMessageBox>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#define MILOGGER_CATEGORY "kvhqc.MainDialog"
#include "HqcLogging.hh"

using namespace Helpers;

namespace {
const char SETTING_WATCHRR_GEOMETRY[] = "geometry_watchrr";
} // anonymous namespace

MainDialog::MainDialog(EditAccessPtr da, ModelAccessPtr ma, const Sensor& sensor, const TimeRange& time, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::DialogMain)
    , mDianaHelper(0)
    , mDA(da)
    , mSensor(sensor)
    , mTime(time)
    , mRRModel(new MainTableModel(mDA, ma, mSensor, mTime))
    , mNeighborModel(new NeighborTableModel(mDA, mSensor, mTime))
    , mNeighborData(new NeighborDataModel(mDA, mSensor, mTime))
{
    ui->setupUi(this);
    {
        QSettings settings;
        if (not restoreGeometry(settings.value(SETTING_WATCHRR_GEOMETRY).toByteArray()))
            METLIBS_LOG_WARN("cannot restore WatchRR geometry");
    }

    QString info = tr("Station %1 [%2]").arg(mSensor.stationId).arg(mSensor.typeId);
    ui->labelStationInfo->setText(info);
    ui->labelInfoRR->setText("");

    ClientButton* cb = new ClientButton("WatchRR2", "/usr/bin/coserver4", ui->tabNeighborData);
    ui->neighborDataButtonLayout->insertWidget(1, cb);
    mDianaHelper.reset(new DianaHelper(cb));

    show();

    qApp->processEvents();
    BusyIndicator wait;

    initializeRR24Data();

    try {
        const kvalobs::kvStation& s = KvMetaDataBuffer::instance()->findStation(mSensor.stationId);
        info += " " + Helpers::stationName(s);
        if (s.environmentid() == 10)
            info += " " + tr("[not daily]");
    } catch(std::runtime_error&) {
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
    ui->labelInfoRR->setText("");
    ui->buttonUndo->setEnabled(false);
    ui->buttonRedo->setEnabled(false);

    ui->tableNeighborRR->setModel(mNeighborModel.get());
    ui->tableNeighborRR->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
    ui->tableNeighborRR->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    ui->tableNeighborRR->verticalHeader()->setFont(mono);
    qApp->processEvents();

    ui->tableNeighborData->setModel(mNeighborData.get());
    ui->tableNeighborData->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
    ui->tableNeighborData->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    qApp->processEvents();

    const boost::gregorian::date d0 = time.t0().date(), d1 = time.t1().date();
    ui->dateNeighborData->setMinimumDate(QDate(d0.year(), d0.month(), d0.day()));
    ui->dateNeighborData->setMaximumDate(QDate(d1.year(), d1.month(), d1.day()));

    connect(ui->tableRR->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this, SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));
    connect(mRRModel.get(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(onDataChanged(const QModelIndex&,const QModelIndex&)));
    connect(mNeighborData.get(), SIGNAL(timeChanged(const timeutil::ptime&)),
            this, SLOT(onNeighborDataTimeChanged(const timeutil::ptime&)));
    connect(mDianaHelper.get(), SIGNAL(receivedTime(const timeutil::ptime&)),
            this, SLOT(onNeighborDataTimeChanged(const timeutil::ptime&)));
    connect(mDianaHelper.get(), SIGNAL(connection(bool)),
            this, SLOT(dianaConnection(bool)));

    mDA->backendDataChanged.connect(boost::bind(&MainDialog::onBackendDataChanged, this, _1, _2));

    mDianaHelper->tryConnect();
    mNeighborData->setTime(time.t1()-boost::posix_time::hours(24));
}

MainDialog::~MainDialog()
{
    mDianaHelper.reset(0);
    mDA->backendDataChanged.disconnect(boost::bind(&MainDialog::onBackendDataChanged, this, _1, _2));

    QSettings settings;
    settings.setValue(SETTING_WATCHRR_GEOMETRY, saveGeometry());
}

void MainDialog::initializeRR24Data()
{
    mEditableTime = mTime;
    mDA->newVersion();
    RR24::analyse(mDA, mSensor, mEditableTime);
        qApp->processEvents();
    FCC::analyse(mDA, mSensor, mEditableTime);
        qApp->processEvents();
    mRRModel->setRR24TimeRange(mEditableTime);
        qApp->processEvents();
}

void MainDialog::onSelectionChanged(const QItemSelection&, const QItemSelection&)
{
    const Selection sel = findSelection();
    if (sel.empty()) {
        ui->labelInfoRR->setText("");
        ui->buttonEdit->setEnabled(false);
        ui->buttonRedist->setEnabled(false);
        ui->buttonRedistQC2->setEnabled(false);
        ui->buttonAcceptRow->setEnabled(false);
        return;
    }

    const int nDays = sel.selTime.days() + 1;

    if (isRR24Selection(sel)) {
        ui->buttonEdit->setEnabled(true);
        ui->buttonAcceptRow->setEnabled(RR24::canAccept(mDA, mSensor, sel.selTime));
        if (nDays <= 1) {
            ui->labelInfoRR->setText("");
            ui->buttonRedist->setEnabled(false);
        } else {
            const float sum = RR24::calculateSum(mDA, mSensor, sel.selTime);
            ui->labelInfoRR->setText(tr("Sum: %1mm").arg(QString::number(sum, 'f', 1)));
            ui->buttonRedist->setEnabled(true);
        }
        ui->buttonRedistQC2->setEnabled(RR24::canRedistributeInQC2(mDA, mSensor, sel.selTime));
    } else {
        ui->labelInfoRR->setText("");
        ui->buttonEdit->setEnabled(false);
        ui->buttonRedist->setEnabled(false);
        ui->buttonRedistQC2->setEnabled(false);
        ui->buttonAcceptRow->setEnabled(isCompleteSingleRowSelection(sel));
    }
}

bool MainDialog::isRR24Selection(const Selection& sel) const
{
    return (sel.minCol == mRRModel->getRR24Column() and sel.minCol == sel.maxCol);
}

bool MainDialog::isCompleteSingleRowSelection(const Selection& sel) const
{
    return (sel.selTime.t0() == sel.selTime.t1())
        and (sel.minCol == 0 and sel.maxCol >= mRRModel->columnCount(QModelIndex()) - 2);
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
    if (isRR24Selection(sel)) {
        RR24::accept(mDA, mSensor, sel.selTime);
    } else if (isCompleteSingleRowSelection(sel)) {
        FCC::acceptRow(mDA, mSensor, sel.selTime.t0());
    } else {
        // should not happen, button is disabled
        return;
    }
    ui->tableRR->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    enableSave();
    clearSelection();
}

void MainDialog::reject()
{
  if (Helpers::askDiscardChanges(mDA->countU(), this))
    QDialog::reject();
}

void MainDialog::onEdit()
{
    const Selection sel = findSelection();
    EditAccessPtr eda = boost::make_shared<EditAccess>(mDA);
    EditDialog edit(this, eda, mSensor, sel.selTime, mEditableTime);
    if (edit.exec()) {
        mDA->newVersion();
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
        mDA->newVersion();
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
    if (mDA->canUndo() and (mDA->currentVersion() > 1)) {
        mDA->undoVersion();
        enableSave();
        clearSelection();
    }
}

void MainDialog::onRedo()
{
  if (mDA->canRedo()) {
    mDA->redoVersion();
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
    METLIBS_LOG_SCOPE();
    const int updates = mDA->countU(), tasks = mDA->countT();
    METLIBS_LOG_DEBUG(LOGVAL(updates) << LOGVAL(tasks));

    ui->buttonSave->setEnabled(tasks == 0 and updates > 0);
    ui->buttonUndo->setEnabled(mDA->canUndo() and mDA->currentVersion() > 1);
    ui->buttonRedo->setEnabled(mDA->canRedo());
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
    BusyIndicator wait;
    QDateTime qdt(date, QTime(mTime.t0().time_of_day().hours(), 0, 0));
    const timeutil::ptime& time = timeutil::from_QDateTime(qdt);
    mNeighborData->setTime(time);
    mDianaHelper->sendTime(time);
    ui->tableNeighborData->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

void MainDialog::onNeighborDataTimeChanged(const timeutil::ptime& time)
{
    ui->dateNeighborData->setDateTime(timeutil::to_QDateTime(time));
    mDianaHelper->sendTime(time);
    mNeighborData->setTime(time);
}

void MainDialog::dianaConnection(bool c)
{
    if (not c)
        return;

    std::vector<timeutil::ptime> times;
    for(timeutil::ptime t = mTime.t0(); t <= mTime.t1(); t += boost::posix_time::hours(24))
        times.push_back(t);
    mDianaHelper->sendTimes(times);
    mDianaHelper->sendTime(mNeighborData->getTime());

    const std::vector<int> stations = mNeighborData->neighborStations();
    mDianaHelper->sendStations(stations);
}
