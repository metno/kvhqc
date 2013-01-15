
#ifndef DATARR24ITEM_HH
#define DATARR24ITEM_HH 1

#include "DataCorrectedItem.hh"

class DataRR24Item : public DataCorrectedItem {
public:
    DataRR24Item(bool showNew, Code2TextPtr codes);

    virtual Qt::ItemFlags flags() const;
    virtual QVariant data(EditDataPtr obs, int role) const;
    virtual bool setData(EditDataPtr obs, EditAccessPtr da, const SensorTime& st, const QVariant& value, int role);
};

#endif // DATARR24ITEM_HH
