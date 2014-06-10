
#ifndef COMMON_PARAMQUERYTASK_HH
#define COMMON_PARAMQUERYTASK_HH 1

#include "QueryTask.hh"
#include <kvalobs/kvParam.h>
#include <QtCore/QObject>
#include <vector>

class ParamQueryTask : public QObject, public QueryTask
{ Q_OBJECT;
public:
  typedef std::vector<kvalobs::kvParam> kvParam_v;

  ParamQueryTask(size_t priority);
  
  QString querySql(QString dbversion) const;
  void notifyRow(const ResultRow& row);
  void notifyStatus(int status);

  const kvParam_v& params() const
    { return mParams; }

Q_SIGNALS:
  void queryStatus(int);

private:
  kvParam_v mParams;
};

#endif // COMMON_PARAMQUERYTASK_HH
