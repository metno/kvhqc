
#ifndef DATACORRECTEDITEM_HH
#define DATACORRECTEDITEM_HH 1

#include "Code2Text.hh"
#include "DataItem.hh"

class DataCorrectedItem : public DataItem {
public:
    DataCorrectedItem(bool showNew, Code2TextPtr codes);

    virtual Qt::ItemFlags flags(EditDataPtr obs) const;
    virtual QVariant data(EditDataPtr obs, int role) const;
    virtual bool setData(EditDataPtr obs, EditAccessPtr da, const SensorTime& st, const QVariant& value, int role);
    virtual QString description(bool mini) const;
    virtual int type() const
    { return mShowNew ? ObsColumn::NEW_CORRECTED : ObsColumn::OLD_CORRECTED; }

protected:
    float getValue(EditDataPtr obs) const;

protected:
    Code2TextPtr mCodes;
    bool mShowNew;
};

typedef boost::shared_ptr<DataCorrectedItem> DataCorrectedItemPtr;

#endif // DATACORRECTEDITEM_HH
