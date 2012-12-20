
#ifndef RR24COLUMN_HH
#define RR24COLUMN_HH 1

#include "DataColumn.hh"

class RR24Column : public DataColumn {
public:
    RR24Column(EditAccessPtr kda, const Sensor& sensor, DisplayType displayType);
    virtual QVariant data(const timeutil::ptime& time, int role) const;
};

#endif // RR24COLUMN_HH
