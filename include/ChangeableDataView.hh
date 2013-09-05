
#ifndef ChangeableDataView_hh
#define ChangeableDataView_hh

#include "DataView.hh"

class ChangeableDataView : public DataView
{
public:
  virtual std::string changes() = 0;
  virtual void replay(const std::string& changes) = 0;
};

#endif // ChangeableDataView_hh
