
#include "WeatherDialog.hh"

#include "BusyIndicator.h"
#include "Helpers.hh"
#include "KvMetaDataBuffer.hh"
#include "WeatherTableModel.hh"
#include "ObsDelegate.hh"

#include <QtGui/QMessageBox>
#include <QtCore/qsettings.h>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

#include "ui_weatherdialog.h"

#define MILOGGER_CATEGORY "kvhqc.WeatherDialog"
#include "HqcLogging.hh"

using namespace Helpers;

namespace {
const char SETTING_WEATHER_GEOMETRY[] = "geometry_weather";
} // anonymous namespace

WeatherDialog::WeatherDialog(EditAccessPtr da, const Sensor& sensor, const TimeRange& time, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::WeatherDialog)
    , mParentDA(da)
    , mDA(boost::make_shared<EditAccess>(mParentDA))
    , mSensor(sensor)
    , mTime(time)
    , mModelCorrected(new WeatherTableModel(mDA, mSensor, mTime, ObsColumn::NEW_CORRECTED))
    , mModelOriginal(new WeatherTableModel(mDA, mSensor, mTime, ObsColumn::ORIGINAL))
    , mModelFlags(new WeatherTableModel(mDA, mSensor, mTime, ObsColumn::NEW_CONTROLINFO))
{
    ui->setupUi(this);
    {
        QSettings settings;
        if (not restoreGeometry(settings.value(SETTING_WEATHER_GEOMETRY).toByteArray()))
            METLIBS_LOG_INFO("cannot restore Weather geometry");
    }

    QString info = tr("Station %1 [%2]").arg(mSensor.stationId).arg(mSensor.typeId);
    ui->labelStationInfo->setText(info);
    ui->labelInfoRR->setText("");

    qApp->processEvents();
    BusyIndicator wait;

    QFont mono("Monospace");

    ui->buttonSave->setEnabled(false);
    ui->tableCorrected->setModel(mModelCorrected.get());
    ui->tableCorrected->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
    ui->tableCorrected->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    ui->tableCorrected->setItemDelegate(new ObsDelegate(this));
    ui->tableCorrected->verticalHeader()->setFont(mono);
    qApp->processEvents();

    ui->tableOriginal->setModel(mModelOriginal.get());
    ui->tableOriginal->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
    ui->tableOriginal->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    ui->tableOriginal->setItemDelegate(new ObsDelegate(this));
    ui->tableOriginal->verticalHeader()->setFont(mono);
    qApp->processEvents();

    ui->tableFlags->setModel(mModelFlags.get());
    ui->tableFlags->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
    ui->tableFlags->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    ui->tableFlags->setItemDelegate(new ObsDelegate(this));
    ui->tableFlags->verticalHeader()->setFont(mono);
    qApp->processEvents();

    ui->buttonUndo->setEnabled(false);
    ui->buttonRedo->setEnabled(false);

    mDA->backendDataChanged.connect(boost::bind(&WeatherDialog::onBackendDataChanged, this, _1, _2));

    connect(mModelCorrected.get(), SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
        this, SLOT(onDataChanged(const QModelIndex&,const QModelIndex&)));

    ui->buttonsAcceptReject->updateModel(mDA, ui->tableCorrected);
}

WeatherDialog::~WeatherDialog()
{
    mDA->backendDataChanged.disconnect(boost::bind(&WeatherDialog::onBackendDataChanged, this, _1, _2));

    QSettings settings;
    settings.setValue(SETTING_WEATHER_GEOMETRY, saveGeometry());
}

void WeatherDialog::reject()
{
  if (Helpers::askDiscardChanges(mDA->countU(), this))
    QDialog::reject();
}

void WeatherDialog::accept()
{
  mParentDA->newVersion();
  if (not mDA->sendChangesToParent(false)) {
    QMessageBox::critical(this,
        windowTitle(),
        tr("Sorry, your changes could not be saved and are lost!"),
        tr("OK"),
        "");
  }
  QDialog::accept();
}

void WeatherDialog::onBackendDataChanged(ObsAccess::ObsDataChange what, EditDataPtr ebs)
{
  METLIBS_LOG_SCOPE();
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
    // ui->buttonsAcceptReject->enableButtons(); // necessary?
}

void WeatherDialog::onUndo()
{
  if (mDA->canUndo()) {
    mDA->undoVersion();
    enableSave();
    clearSelection();
  }
}

void WeatherDialog::onRedo()
{
  if (mDA->canRedo()) {
    mDA->redoVersion();
    enableSave();
    clearSelection();
  }
}

// TODO this is a helper function
void WeatherDialog::clearSelection(QTableView* tv)
{
  QAbstractItemModel* mdl = tv->model();
  const QModelIndex tl = mdl->index(0, 0, QModelIndex());
  const QModelIndex br = mdl->index(mdl->rowCount(QModelIndex())-1,
      mdl->columnCount(QModelIndex())-1, QModelIndex());
  QItemSelection s;
  s.select(tl, br);
  tv->selectionModel()->select(s, QItemSelectionModel::Clear);
}

void WeatherDialog::clearSelection()
{
  clearSelection(ui->tableCorrected);
  clearSelection(ui->tableOriginal);
  clearSelection(ui->tableFlags);
}

void WeatherDialog::enableSave()
{
    METLIBS_LOG_SCOPE();
    int updates = mDA->countU(), tasks = mDA->countT();

    ui->buttonSave->setEnabled(tasks == 0 and updates > 0);
    ui->buttonUndo->setEnabled(mDA->canUndo());
    ui->buttonRedo->setEnabled(mDA->canRedo());
}

void WeatherDialog::onDataChanged(const QModelIndex&, const QModelIndex&)
{
    enableSave();
}

