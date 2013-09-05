
#include "SimpleCorrections.hh"

#include <boost/bind.hpp>

#include "ui_simplecorrections.h"
//#define NDEBUG
#include "debug.hh"

SimpleCorrections::SimpleCorrections(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SimpleCorrections)
    , mSensorTime(Sensor(0, 0, 0, 0, 0), timeutil::ptime())
{
    ui->setupUi(this);
    LOG4HQC_DEBUG("SimpleCorrections", DBG1(ui->gridInfo->minimumSize().width()));
}

SimpleCorrections::~SimpleCorrections()
{
    if (mDA)
        mDA->obsDataChanged.disconnect(boost::bind(&SimpleCorrections::onDataChanged, this, _1, _2));
}

void SimpleCorrections::setDataAccess(EditAccessPtr eda)
{
    if (mDA)
        mDA->obsDataChanged.disconnect(boost::bind(&SimpleCorrections::onDataChanged, this, _1, _2));
    mDA = eda;
    if (mDA)
        mDA->obsDataChanged.connect(boost::bind(&SimpleCorrections::onDataChanged, this, _1, _2));

    navigateTo(mSensorTime);
}

void SimpleCorrections::navigateTo(const SensorTime& st)
{
    mSensorTime = st;

    const Sensor& s = mSensorTime.sensor;
    if (s.stationId > 0) {
        ui->textStation->setText(QString::number(s.stationId));
        ui->textParam->setText(QString::number(s.paramId));
        ui->textType->setText(QString::number(s.typeId));

        ui->textObstime->setText(QString::fromStdString(timeutil::to_iso_extended_string(mSensorTime.time)));
    } else {
        ui->textStation->setText("--");
        ui->textParam->setText("--");
        ui->textType->setText("--");

        ui->textObstime->setText("");
    }
}

void SimpleCorrections::onDataChanged(ObsAccess::ObsDataChange, ObsDataPtr data)
{
    if (data and eq_SensorTime()(data->sensorTime(), mSensorTime))
        navigateTo(mSensorTime);
}
