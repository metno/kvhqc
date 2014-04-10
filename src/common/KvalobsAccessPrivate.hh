
#ifndef ACCESS_KVALOBSACCESSPRIVATE_HH
#define ACCESS_KVALOBSACCESSPRIVATE_HH 1

#include "KvalobsAccess.hh"
#include "KvalobsData.hh"

#include <QtSql/QSqlDatabase>

// ========================================================================

class KvalobsHandler : public BackgroundHandler
{
public:
  virtual void initialize();
  virtual void finalize();

  virtual void queryData(ObsRequest_p request);

private:
  QSqlDatabase mKvalobsDB;
};

#endif // ACCESS_KVALOBSACCESSPRIVATE_HH
