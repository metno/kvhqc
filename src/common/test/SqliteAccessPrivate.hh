
#ifndef ACCESS_SQLITEACCESSPRIVATE_HH
#define ACCESS_SQLITEACCESSPRIVATE_HH 1

#include "SqliteAccess.hh"
#include <sqlite3.h>

class SqliteQueryRunner : public QueryTaskRunner {
public:
  SqliteQueryRunner();
  ~SqliteQueryRunner();

  void initialize() { }
  void finalize() { }

  void run(QueryTask* qtask);
  int exec(const std::string& sql);

private:
  sqlite3_stmt* prepare_statement(const std::string& sql, QueryTask* qtask);
  void finalize_statement(sqlite3_stmt* stmt, int lastStep, QueryTask* qtask);

private:
  sqlite3 *db;
};

HQC_TYPEDEF_P(SqliteQueryRunner);

#endif // ACCESS_SQLITEACCESSPRIVATE_HH
