
#ifndef ACCESS_KVALOBSACCESSPRIVATE_HH
#define ACCESS_KVALOBSACCESSPRIVATE_HH 1

#include "KvalobsAccess.hh"

#include <QtSql/QSqlDatabase>

// ========================================================================

class KvalobsHandler : public BackgroundHandler
{
public:
  virtual void initialize();
  virtual void finalize();

  virtual ObsData_pv queryData(ObsRequest_p request);

private:
  QSqlDatabase mKvalobsDB;
};

#endif // ACCESS_KVALOBSACCESSPRIVATE_HH
