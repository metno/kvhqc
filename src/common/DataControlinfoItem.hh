
#ifndef DATACONTROLINFOITEM_HH
#define DATACONTROLINFOITEM_HH 1

#include "DataItem.hh"

class DataControlinfoItem : public DataItem {
public:
  DataControlinfoItem();
  
  virtual QVariant data(ObsData_p obs, const SensorTime& st, int role) const;
  virtual QString description(bool mini) const;
  virtual int type() const
    { return ObsColumn::NEW_CONTROLINFO; }

protected:
  const kvalobs::kvControlInfo& getControlinfo(ObsData_p obs) const;
};

HQC_TYPEDEF_P(DataControlinfoItem);

#endif // DATACONTROLINFOITEM_HH
