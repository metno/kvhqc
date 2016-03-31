
#ifndef DATACONTROLINFOITEM_HH
#define DATACONTROLINFOITEM_HH 1

#include "DataItem.hh"

class DataControlinfoItem : public DataItem {
public:
    DataControlinfoItem(bool showNew);

    virtual QVariant data(EditDataPtr obs, const SensorTime& st, int role) const;
    virtual QString description(bool mini) const;
    virtual int type() const
      { return mShowNew ? ObsColumn::NEW_CONTROLINFO : ObsColumn::OLD_CONTROLINFO; }

protected:
    kvalobs::kvControlInfo getControlinfo(EditDataPtr obs) const;

protected:
    bool mShowNew;
};

typedef std::shared_ptr<DataControlinfoItem> DataControlinfoItemPtr;

#endif // DATACONTROLINFOITEM_HH
