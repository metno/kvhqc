
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
    , mDA(da)
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
    ui->buttonRedo->setVisible(false);

    mDA->backendDataChanged.connect(boost::bind(&WeatherDialog::onBackendDataChanged, this, _1, _2));
}

WeatherDialog::~WeatherDialog()
{
    mDA->backendDataChanged.disconnect(boost::bind(&WeatherDialog::onBackendDataChanged, this, _1, _2));

    QSettings settings;
    settings.setValue(SETTING_WEATHER_GEOMETRY, saveGeometry());
}

void WeatherDialog::onBackendDataChanged(ObsAccess::ObsDataChange what, EditDataPtr ebs)
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
