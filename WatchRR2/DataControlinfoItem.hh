
#ifndef DATACONTROLINFOITEM_HH
#define DATACONTROLINFOITEM_HH 1

#include "DataItem.hh"

class DataControlinfoItem : public DataItem {
public:
    DataControlinfoItem(bool showNew);

    virtual QVariant data(EditDataPtr obs, int role) const;
    virtual QString description(bool mini) const;

protected:
    kvalobs::kvControlInfo getControlinfo(EditDataPtr obs) const;

protected:
    bool mShowNew;
};

typedef boost::shared_ptr<DataControlinfoItem> DataControlinfoItemPtr;

#endif // DATACONTROLINFOITEM_HH
