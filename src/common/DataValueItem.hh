
#ifndef DATAVALUEITEM_HH
#define DATAVALUEITEM_HH 1

#include "DataItem.hh"

class DataValueItem : public DataItem {
public:
  DataValueItem(ObsColumn::Type columnType);

  virtual Qt::ItemFlags flags(ObsData_p obs) const;
  virtual QVariant data(ObsData_p obs, const SensorTime& st, int role) const;
  virtual int type() const;
  
protected:
  virtual float getValue(ObsData_p obs) const;

protected:
  ObsColumn::Type mColumnType;
};

HQC_TYPEDEF_P(DataValueItem);

#endif // DATAVALUEITEM_HH
