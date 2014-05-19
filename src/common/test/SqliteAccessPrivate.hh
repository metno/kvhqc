
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

  void queryData(ObsRequest_p request);
  void queryTask(QueryTask* qtask);
  int exec(const std::string& sql);

private:
  sqlite3_stmt* prepare_statement(const std::string& sql, QueryTask* qtask);
  void finalize_statement(sqlite3_stmt* stmt, int lastStep, QueryTask* qtask);

private:
  sqlite3 *db;
};

HQC_TYPEDEF_P(SqliteHandler);

#endif // ACCESS_SQLITEACCESSPRIVATE_HH
