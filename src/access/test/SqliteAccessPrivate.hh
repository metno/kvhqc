
#ifndef ACCESS_SQLITEACCESSPRIVATE_HH
#define ACCESS_SQLITEACCESSPRIVATE_HH 1

#include "SqliteAccess.hh"

#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtCore/QWaitCondition>

#include <sqlite3.h>

#include <queue> // priority_queue

// ========================================================================

class SqliteHandler
{
public:
  SqliteHandler();
  ~SqliteHandler();

  ObsData_pv queryData(ObsRequest_p request);
  void exec(const std::string& sql);

private:
  sqlite3_stmt* prepare_statement(const std::string& sql);
  void finalize_statement(sqlite3_stmt* stmt, int lastStep);

private:
  sqlite3 *db;
};

// ========================================================================

class SqliteThread : public QThread
{ Q_OBJECT;
public:
  SqliteThread(SqliteHandler_p sqlh);
  ~SqliteThread();

  void enqueueRequest(ObsRequest_p request);

  virtual void run();

Q_SIGNALS:
  void newData(ObsRequest_p request, const ObsData_pv& data);

private:
  struct QueuedQuery {
    int priority;

    ObsRequest_p request;

    QueuedQuery(int p, ObsRequest_p r)
      : priority(p), request(r) { }
    
    bool operator<(const QueuedQuery& other) const
      { return priority < other.priority; }
  };

  SqliteHandler_p mSqlite;

  typedef std::priority_queue<QueuedQuery> QueryQueue_t;
  QueryQueue_t mQueue;

  QMutex mMutex;
  QWaitCondition mCondition;
  bool mDone;
};

#endif // ACCESS_SQLITEACCESSPRIVATE_HH
