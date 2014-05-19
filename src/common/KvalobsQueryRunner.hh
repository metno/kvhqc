
#ifndef ACCESS_KVALOBSACCESSPRIVATE_HH
#define ACCESS_KVALOBSACCESSPRIVATE_HH 1

#include "QueryTaskHandler.hh"

#include <QtSql/QSqlDatabase>

// ========================================================================

class KvalobsQueryRunner : public QueryTaskRunner
{
public:
  void initialize();
  void finalize();
  void run(QueryTask* task);

private:
  QSqlDatabase mKvalobsDB;
};

HQC_TYPEDEF_P(KvalobsQueryRunner);

#endif // ACCESS_KVALOBSACCESSPRIVATE_HH
