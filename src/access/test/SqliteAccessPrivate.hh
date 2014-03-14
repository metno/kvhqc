
#ifndef ACCESS_SQLITEACCESSPRIVATE_HH
#define ACCESS_SQLITEACCESSPRIVATE_HH 1

#include "SqliteAccess.hh"
#include <sqlite3.h>

class SqliteHandler : public BackgroundHandler {
public:
  SqliteHandler();
  ~SqliteHandler();

  virtual void initialize() { }
  virtual void finalize() { }

  ObsData_pv queryData(ObsRequest_p request);
  void exec(const std::string& sql);

private:
  sqlite3_stmt* prepare_statement(const std::string& sql);
  void finalize_statement(sqlite3_stmt* stmt, int lastStep);

private:
  sqlite3 *db;
};

HQC_TYPEDEF_P(SqliteHandler);

#endif // ACCESS_SQLITEACCESSPRIVATE_HH
