
#include "WatchRRDialog.hh"

#include "common/DianaHelper.hh"
#include "EditDialog.hh"
#include "StationCardModel.hh"
#include "NeighborCardsModel.hh"
#include "NeighborRR24Model.hh"
#include "RedistDialog.hh"
#include "common/AnalyseFCC.hh"
#include "common/AnalyseRR24.hh"
#include "common/KvMetaDataBuffer.hh"
#include "common/gui/ObsDelegate.hh"
#include "util/Helpers.hh"
#include "util/gui/BusyIndicator.hh"
#include "util/gui/UiHelpers.hh"

#include "ui_watchrr_main.h"
#include "ui_watchrr_redist.h"

#include <kvalobs/kvDataOperations.h>
#ifdef METLIBS_BEFORE_4_9_5
#define signals Q_SIGNALS
#define slots Q_SLOTS
#endif
#include <coserver/ClientButton.h>
#include <QtCore/qsettings.h>
#include <QtGui/QMessageBox>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <memory>

#define MILOGGER_CATEGORY "kvhqc.WatchRRDialog"
#include "util/HqcLogging.hh"

using namespace Helpers;

namespace {
const char SETTING_WATCHRR_GEOMETRY[] = "geometry_watchrr";
} // anonymous namespace

WatchRRDialog::WatchRRDialog(EditAccessPtr da, ModelAccessPtr ma, const Sensor& sensor, const TimeRange& time, QWidget* parent)
  : QDialog(parent)
  , ui(new Ui::DialogMain)
  , mDianaHelper(0)
  , mParentDA(da)
  , mDA(std::make_shared<EditAccess>(mParentDA))
  , mSensor(sensor)
  , mTime(time)
  , mStationCard(new StationCardModel(mDA, ma, mSensor, mTime))
  , mNeighborCards(new NeighborCardsModel(mDA, mSensor, mTime))
{
  ui->setupUi(this);
  {
    QSettings settings;
    if (not restoreGeometry(settings.value(SETTING_WATCHRR_GEOMETRY).toByteArray()))
      HQC_LOG_WARN("cannot restore WatchRR geometry");
  }

  setStationInfoText();

  ClientButton* cb = new ClientButton("WatchRR2", "/usr/bin/coserver4", ui->tabNeighborCards);
  ui->neighborDataButtonLayout->insertWidget(1, cb);
  mDianaHelper.reset(new DianaHelper(cb));

  show();

  Helpers::processNonUserEvents();
  BusyIndicator wait;

  initializeRR24Data();

  QFont mono("Monospace");

  ui->buttonSave->setEnabled(false);
  ui->tableStationCard->setModel(mStationCard.get());
  ui->tableStationCard->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
  ui->tableStationCard->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
  ui->tableStationCard->setItemDelegate(new ObsDelegate(this));
  ui->tableStationCard->verticalHeader()->setFont(mono);
  ui->labelInfoRR->setText("");
  ui->buttonUndo->setEnabled(false);
  ui->buttonRedo->setEnabled(false);
  Helpers::processNonUserEvents();

  ui->tableNeighborRR24->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
  ui->tableNeighborRR24->verticalHeader()->setFont(mono);
  Helpers::processNonUserEvents();

  ui->tableNeighborCards->setModel(mNeighborCards.get());
  ui->tableNeighborCards->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
  ui->tableNeighborCards->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
  Helpers::processNonUserEvents();

  const boost::gregorian::date d0 = time.t0().date(), d1 = time.t1().date();
  ui->dateNeighborCards->setMinimumDate(QDate(d0.year(), d0.month(), d0.day()));
  ui->dateNeighborCards->setMaximumDate(QDate(d1.year(), d1.month(), d1.day()));

  connect(ui->tableStationCard->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
      this, SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection&)));
  connect(mStationCard.get(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
      this, SLOT(onDataChanged(const QModelIndex&,const QModelIndex&)));
  connect(mNeighborCards.get(), SIGNAL(timeChanged(const timeutil::ptime&)),
      this, SLOT(onNeighborDataTimeChanged(const timeutil::ptime&)));
  connect(mDianaHelper.get(), SIGNAL(receivedTime(const timeutil::ptime&)),
      this, SLOT(onNeighborDataTimeChanged(const timeutil::ptime&)));
  connect(mDianaHelper.get(), SIGNAL(connection(bool)),
      this, SLOT(dianaConnection(bool)));

  mDA->backendDataChanged.connect(boost::bind(&WatchRRDialog::onBackendDataChanged, this, _1, _2));

  mDianaHelper->tryConnect();
  mNeighborCards->setTime(time.t1()-boost::posix_time::hours(24));
}

WatchRRDialog::~WatchRRDialog()
{
  mDianaHelper.reset(0);
  mDA->backendDataChanged.disconnect(boost::bind(&WatchRRDialog::onBackendDataChanged, this, _1, _2));

  QSettings settings;
  settings.setValue(SETTING_WATCHRR_GEOMETRY, saveGeometry());
}

void WatchRRDialog::setStationInfoText()
{
  QString info = tr("Station %1 [%2]").arg(mSensor.stationId).arg(mSensor.typeId);
  try {
    const kvalobs::kvStation& s = KvMetaDataBuffer::instance()->findStation(mSensor.stationId);
    info += " " + Helpers::stationName(s);
    if (s.environmentid() == 10)
      info += " " + tr("[not daily]");
  } catch (std::exception&) {
    // TODO handle errors
  }
  ui->labelStationInfo->setText(info);
}

void WatchRRDialog::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::LanguageChange) {
    ui->retranslateUi(this);
    setStationInfoText();
    onSelectionChanged(QItemSelection(), QItemSelection()); // updates ui->labelInfoRR
  }
  QDialog::changeEvent(event);
}

void WatchRRDialog::initializeRR24Data()
{
  mEditableTime = mTime;
  mDA->newVersion();
  RR24::analyse(mDA, mSensor, mEditableTime);
  Helpers::processNonUserEvents();
  FCC::analyse(mDA, mSensor, mEditableTime);
  Helpers::processNonUserEvents();
  mStationCard->setRR24TimeRange(mEditableTime);
  Helpers::processNonUserEvents();
}

void WatchRRDialog::onSelectionChanged(const QItemSelection&, const QItemSelection&)
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

bool WatchRRDialog::isRR24Selection(const Selection& sel) const
{
  return (sel.minCol == mStationCard->getRR24Column() and sel.minCol == sel.maxCol);
}

bool WatchRRDialog::isCompleteSingleRowSelection(const Selection& sel) const
{
  return (sel.selTime.t0() == sel.selTime.t1())
      and (sel.minCol == 0 and sel.maxCol >= mStationCard->columnCount(QModelIndex()) - 2);
}

WatchRRDialog::Selection WatchRRDialog::findSelection()
{
  QModelIndexList selected = ui->tableStationCard->selectionModel()->selectedIndexes();
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
  s.selTime = TimeRange(mStationCard->timeAtRow(minRow), mStationCard->timeAtRow(maxRow));
  return s;
}

void WatchRRDialog::onAcceptRow()
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
  ui->tableStationCard->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
  enableSave();
  clearSelection();
}

void WatchRRDialog::reject()
{
  if (Helpers::askDiscardChanges(mDA->countU(), this))
    QDialog::reject();
}

void WatchRRDialog::accept()
{
  mParentDA->newVersion();
  // FIXME this is a hack to avoid complaints about data changes in parent
  mDA->backendDataChanged.disconnect(boost::bind(&WatchRRDialog::onBackendDataChanged, this, _1, _2));
  if (not mDA->sendChangesToParent(false)) {
    QMessageBox::critical(this,
        windowTitle(),
        tr("Sorry, your changes could not be saved and are lost!"),
        tr("OK"),
        "");
  }
  QDialog::accept();
}

void WatchRRDialog::onEdit()
{
  const Selection sel = findSelection();
  EditAccessPtr eda = std::make_shared<EditAccess>(mDA);
  EditDialog edit(this, eda, mSensor, sel.selTime, mEditableTime);
  if (edit.exec()) {
    mDA->newVersion();
    eda->sendChangesToParent();
    ui->tableStationCard->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    enableSave();
    clearSelection();
  }
}

void WatchRRDialog::onRedistribute()
{
  const Selection sel = findSelection();
  EditAccessPtr eda = std::make_shared<EditAccess>(mDA);
  RedistDialog redist(this, eda, mSensor, sel.selTime, mEditableTime);
  if (redist.exec()) {
    mDA->newVersion();
    eda->sendChangesToParent();
    ui->tableStationCard->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    enableSave();
    clearSelection();
  }
}

void WatchRRDialog::onRedistributeQC2()
{
  const Selection sel = findSelection();
  RR24::redistributeInQC2(mDA, mSensor, sel.selTime, mEditableTime);
  ui->tableStationCard->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
  enableSave();
  clearSelection();
}

void WatchRRDialog::onUndo()
{
  if (mDA->canUndo() and (mDA->currentVersion() > 1)) {
    mDA->undoVersion();
    enableSave();
    clearSelection();
  }
}

void WatchRRDialog::onRedo()
{
  if (mDA->canRedo()) {
    mDA->redoVersion();
    enableSave();
    clearSelection();
  }
}

void WatchRRDialog::clearSelection()
{
  const QModelIndex tl = mStationCard->index(0, 0, QModelIndex());
  const QModelIndex br = mStationCard->index(mStationCard->rowCount(QModelIndex())-1,
      mStationCard->columnCount(QModelIndex())-1, QModelIndex());
  QItemSelection s;
  s.select(tl, br);
  ui->tableStationCard->selectionModel()->select(s, QItemSelectionModel::Clear);
}

void WatchRRDialog::enableSave()
{
  METLIBS_LOG_SCOPE();
  const int updates = mDA->countU(), tasks = mDA->countT();
  METLIBS_LOG_DEBUG(LOGVAL(updates) << LOGVAL(tasks));

  ui->buttonSave->setEnabled(tasks == 0 and updates > 0);
  ui->buttonUndo->setEnabled(mDA->canUndo() and mDA->currentVersion() > 1);
  ui->buttonRedo->setEnabled(mDA->canRedo());
}

void WatchRRDialog::onDataChanged(const QModelIndex&, const QModelIndex&)
{
  enableSave();
}

void WatchRRDialog::onBackendDataChanged(ObsAccess::ObsDataChange what, EditDataPtr ebs)
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

void WatchRRDialog::onNeighborDataDateChanged(const QDate& date)
{
  BusyIndicator wait;
  QDateTime qdt(date, QTime(mTime.t0().time_of_day().hours(), 0, 0));
  const timeutil::ptime& time = timeutil::from_QDateTime(qdt);
  mNeighborCards->setTime(time);
  mDianaHelper->sendTime(time);
  ui->tableNeighborCards->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

void WatchRRDialog::onNeighborDataTimeChanged(const timeutil::ptime& time)
{
  ui->dateNeighborCards->setDateTime(timeutil::to_QDateTime(time));
  mDianaHelper->sendTime(time);
  mNeighborCards->setTime(time);
}

void WatchRRDialog::onCurrentTabChanged(int tab)
{
  if (tab == 1 and not mNeighborRR24.get()) {
    mNeighborRR24.reset(new NeighborRR24Model(mDA, mSensor, mTime));
    ui->tableNeighborRR24->setModel(mNeighborRR24.get());
    ui->tableNeighborRR24->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
  }
}

void WatchRRDialog::dianaConnection(bool c)
{
  if (not c)
    return;

  std::vector<timeutil::ptime> times;
  for(timeutil::ptime t = mTime.t0(); t <= mTime.t1(); t += boost::posix_time::hours(24))
    times.push_back(t);
  mDianaHelper->sendTimes(times);
  mDianaHelper->sendTime(mNeighborCards->getTime());

  const std::vector<int> stations = mNeighborCards->neighborStations();
  mDianaHelper->sendStations(stations);
}
