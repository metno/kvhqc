
#include "StationDialog.hh"

#include "AnalyseRR24.hh"
#include "RedistTableModel.hh"

#include "ui_watchrr_station.h"

#include <kvalobs/kvObsPgm.h>
#include <kvcpp/KvApp.h>

#include <boost/foreach.hpp>

#define NDEBUG
#include "debug.hh"

static const int MIN_DAYS = 7, MAX_DAYS = 120;

StationDialog::StationDialog(const Sensor& sensor, const TimeRange& time, QDialog* parent)
  : QDialog(parent)
  , mSensor(sensor)
  , mHour(-1)
{
    init();

    ui->editStation->setText(QString::number(mSensor.stationId));
    const boost::gregorian::date d0 = time.t0().date();
    ui->dateFrom->setDate(QDate(d0.year(), d0.month(), d0.day()));
    const boost::gregorian::date d1 = time.t1().date();
    ui->dateTo->setDate(QDate(d1.year(), d1.month(), d1.day()));

    onUpdateType();
}

StationDialog::StationDialog(QDialog* parent)
  : QDialog(parent)
  , mSensor(-1, kvalobs::PARAMID_RR_24, 0, 0, -1)
  , mHour(-1)
{
    init();
    onUpdateType();
}

void StationDialog::init()
{
    LOG_SCOPE();
    ui.reset(new Ui::DialogStation);
    ui->setupUi(this);

    ui->labelStationInfo->setText("");

    QDate today = QDate::currentDate();
    ui->dateFrom->setMinimumDate(today.addDays(-MAX_DAYS));
    ui->dateFrom->setMaximumDate(today.addDays(365-MIN_DAYS));
    ui->dateTo->setMinimumDate(today.addDays(-MAX_DAYS+MIN_DAYS));
    ui->dateTo->setMaximumDate(today.addDays(365));
    ui->dateFrom->setDate(today.addDays(-4*MIN_DAYS));
    ui->dateTo->setDate(today);
}

StationDialog::~StationDialog()
{
    LOG_SCOPE();
}

void StationDialog::onUpdateType()
{
    const bool ok = check();
    ui->buttonOK->setEnabled(ok);

    QString typeText;
    if (mSensor.typeId >= 0)
        typeText = QString::number(mSensor.typeId);
    else
        typeText = tr("invalid station or date range");
    ui->labelTypeInfo->setText(typeText);
}

bool StationDialog::check()
{
    bool ok = false;
    const int stationId = ui->editStation->text().toInt(&ok);
    if (not ok or stationId < 60 or stationId > 100000) {
        ui->labelStationInfo->setText(tr("Cannot read station number"));
        return false;
    }

    if (ui->dateTo->date() < ui->dateFrom->date().addDays(MIN_DAYS)) {
        const QDate fromPlusMIN = ui->dateFrom->date().addDays(MIN_DAYS);
        if (fromPlusMIN <= ui->dateTo->maximumDate())
            ui->dateTo->setDate(fromPlusMIN);
        else
            ui->dateFrom->setDate(ui->dateTo->date().addDays(-MIN_DAYS));
    }

    mSensor.stationId = stationId;
    std::list<long> stations(1, mSensor.stationId);
    std::list<kvalobs::kvObsPgm> obs_pgm;
    if (not kvservice::KvApp::kvApp->getKvObsPgm(obs_pgm, stations, false)) {
        ui->labelStationInfo->setText(tr("problem loading obs_pgm"));
        return false;
    }

    mTimeFrom = timeutil::from_QDateTime(QDateTime(ui->dateFrom->date()));
    mTimeTo   = timeutil::from_QDateTime(QDateTime(ui->dateTo  ->date()));
    int typeFrom = -1, typeTo = -1;
    timeutil::ptime tFromEnd, tToStart;
    BOOST_FOREACH (const kvalobs::kvObsPgm& op, obs_pgm) {
        if (op.paramID() == kvalobs::PARAMID_RR_24
            and (op.typeID() == 302 or op.typeID() == 402)
            and (op.kl06() or op.kl07() or op.collector()))
        {
            const timeutil::ptime oFrom = timeutil::from_miTime(op.fromtime()), oTo = timeutil::from_miTime(op.totime());
            if (oFrom <= mTimeFrom and (oTo.is_not_a_date_time() or mTimeFrom <= oTo)) {
                typeFrom = op.typeID();
                tFromEnd = oTo;
                mHour = op.kl07() ? 7 : 6;
            }
            if (oFrom <= mTimeTo and (oTo.is_not_a_date_time() or mTimeTo <= oTo)) {
                typeTo = op.typeID();
                tToStart = oFrom;
                mHour = op.kl07() ? 7 : 6;
            }
        }
    }

    if (typeFrom < 0 or typeTo < 0) {
        ui->labelStationInfo->setText(tr("could not find typeid"));
        return false;
    }

    if (typeFrom != typeTo) {
        ui->labelStationInfo->setText(tr("typeid different at start and end"));
        return false;
    } else {
        mSensor.typeId = typeFrom; // same as typeTo
    }
    ui->labelStationInfo->setText("");
    return true;
}

bool StationDialog::valid() const
{
    return mSensor.stationId >= 60 and mSensor.stationId <= 99999
        and mSensor.typeId >= 1
        and mHour >= 0 and mHour <= 23
        and mTimeFrom <= mTimeTo;
}

void StationDialog::onButtonOk()
{
    LOG_SCOPE();
    if (valid())
        accept();
}

TimeRange StationDialog::selectedTime() const
{
    boost::posix_time::time_duration h = boost::posix_time::hours(mHour);
    return TimeRange(mTimeFrom + h, mTimeTo + h);
 }

