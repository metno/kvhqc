
#ifndef StationDataList_hh
#define StationDataList_hh 1

#include "DynamicDataList.hh"

class StationDataList : public DynamicDataList
{
public:
  StationDataList(QWidget* parent=0);
  ~StationDataList();
  
protected:
  std::string viewType() const;
  DataListModel* makeModel();

private:
  void addSensorColumns(DataListModel* model, const Sensor& s);
};

#endif // StationDataList_hh
