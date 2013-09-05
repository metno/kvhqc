
#ifndef DATARR24ITEM_HH
#define DATARR24ITEM_HH 1

#include "DataCorrectedItem.hh"

class DataRR24Item : public DataCorrectedItem {
public:
    DataRR24Item(bool showNew, Code2TextCPtr codes);

    virtual Qt::ItemFlags flags(EditDataPtr obs) const;
    virtual QVariant data(EditDataPtr obs, int role) const;
};

#endif // DATARR24ITEM_HH
