
#include "SimpleCorrections.hh"

#include <boost/bind.hpp>

#include "ui_simplecorrections.h"
//#define NDEBUG
#include "debug.hh"

static int preferredWidth(QWidget* w)
{ return w->sizeHint().width(); }

static void setCommonMinWidth(QWidget* w[])
{
    int mw = preferredWidth(w[0]);
    for (int i=1; w[i]; ++i)
        mw = std::max(mw, preferredWidth(w[i]));
    for (int i=0; w[i]; ++i)
        w[i]->setMinimumSize(mw, w[i]->minimumSize().height());
}

SimpleCorrections::SimpleCorrections(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SimpleCorrections)
    , mSensorTime(Sensor(0, 0, 0, 0, 0), timeutil::ptime())
{
    ui->setupUi(this);

    QWidget* labels1[] = { ui->labelStation, ui->labelObstime, ui->labelFlags, ui->labelOriginal, 0 };
    setCommonMinWidth(labels1);
    QWidget* labels2[] = { ui->labelType, ui->labelParam, 0 };
    setCommonMinWidth(labels2);

    setMaximumSize(QSize(minimumSize().width(), maximumSize().height()));
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
