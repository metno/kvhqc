
#ifndef STATIONDIALOG_HH
#define STATIONDIALOG_HH

#include "Sensor.hh"
#include "TimeRange.hh"
#include <QtGui/QDialog>
#include <memory>

namespace Ui {
class DialogStation;
}

class StationDialog : public QDialog
{   Q_OBJECT;
public:
    StationDialog(const Sensor& sensor, const TimeRange& time, QDialog* parent=0);
    StationDialog(QDialog* parent=0);
    virtual ~StationDialog();
                            
    Sensor selectedSensor() const
        { return mSensor; }

    TimeRange selectedTime() const;

    bool valid() const;

private Q_SLOTS:
    void init();
    void onButtonOk();
    void onUpdateType();
    bool check();

private:
    Sensor mSensor;
    timeutil::ptime mTimeFrom, mTimeTo;
    int mHour;
    std::auto_ptr<Ui::DialogStation> ui;
};

#endif // STATIONDIALOG_HH
