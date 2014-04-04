
#ifndef DATARR24ITEM_HH
#define DATARR24ITEM_HH 1

#include "DataCorrectedItem.hh"

class DataRR24Item : public DataCorrectedItem {
public:
    DataRR24Item(Code2TextCPtr codes);

    virtual Qt::ItemFlags flags(ObsData_p obs) const;
    virtual QVariant data(ObsData_p obs, const SensorTime& st, int role) const;
};

#endif // DATARR24ITEM_HH
