
#ifndef DATAORIGINALITEM_HH
#define DATAORIGINALITEM_HH 1

#include "Code2Text.hh"
#include "DataItem.hh"

class DataOriginalItem : public DataItem {
public:
    DataOriginalItem(Code2TextPtr codes);

    virtual QVariant data(EditDataPtr obs, int role) const;
    virtual QString description(bool mini) const;

protected:
    float getValue(EditDataPtr obs) const;

protected:
    Code2TextPtr mCodes;
};

typedef boost::shared_ptr<DataOriginalItem> DataOriginalItemPtr;

#endif // DATAORIGINALITEM_HH
