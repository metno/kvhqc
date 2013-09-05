
#ifndef AutoColumnView_hh
#define AutoColumnView_hh 1

#include "DataView.hh"
#include "DataList.hh"

class AutoColumnViewModel;

class AutoColumnView : public DataView
{
public:
    typedef DataList* ViewP;

public:
    AutoColumnView();
    ~AutoColumnView();

    virtual void navigateTo(const SensorTime&);

    void attachView(ViewP v);
    void detachView(ViewP v);

private:
    typedef std::map<Sensor, std::string, lt_Sensor> Changes4ST_t;
    struct ViewInfo {
        ViewP view;
        Changes4ST_t changes;
        ViewInfo(ViewP v) : view(v) { }
        //ViewInfo() : view(0) { }
    };
    typedef std::vector<ViewInfo> Views_t;

private:
    Sensors_t defaultSensors();
    TimeRange defaultTimeLimits();

private:
    Views_t mViews;
    SensorTime mSensorTime;
};

#endif // AutoColumnView_hh
