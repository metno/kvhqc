
#ifndef DATAVXITEM_HH
#define DATAVXITEM_HH 1

#include "DataItem.hh"

class DataVxItem : public DataItem {
public:
    DataVxItem(EditAccessPtr da);

    virtual Qt::ItemFlags flags() const;
    virtual QVariant data(EditDataPtr obs, int role) const;
    virtual bool setData(EditDataPtr obs, EditAccessPtr da, const SensorTime& st, const QVariant& value, int role);
    virtual QString description(bool mini) const;

private:
    typedef std::pair<int,int> Codes_t;
    Codes_t getCodes(EditDataPtr obs1, EditDataPtr obs2) const;

private:
    EditAccessPtr mDA;
};

#endif // DATAVXITEM_HH
