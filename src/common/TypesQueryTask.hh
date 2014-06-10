
#ifndef COMMON_TYPESQUERYTASK_HH
#define COMMON_TYPESQUERYTASK_HH 1

#include "QueryTask.hh"
#include <kvalobs/kvTypes.h>
#include <QtCore/QObject>
#include <vector>

class TypesQueryTask : public QObject, public QueryTask
{ Q_OBJECT;
public:
  typedef std::vector<kvalobs::kvTypes> kvTypes_v;

  TypesQueryTask(size_t priority);
  
  QString querySql(QString dbversion) const;
  void notifyRow(const ResultRow& row);
  void notifyStatus(int status);

  const kvTypes_v& types() const
    { return mTypes; }

Q_SIGNALS:
  void queryStatus(int);

private:
  kvTypes_v mTypes;
};

#endif // COMMON_TYPESQUERYTASK_HH
