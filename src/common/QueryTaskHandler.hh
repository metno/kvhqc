
#ifndef COMMON_QUERYTASKHANDLER_HH
#define COMMON_QUERYTASKHANDLER_HH 1

#include "ObsAccess.hh"
#include <QtCore/QSemaphore>
#include <string>

class QueryTask;

class QueryTaskRunner
{
public:
  virtual ~QueryTaskRunner();
  virtual void initialize() = 0;
  virtual void finalize() = 0;
  virtual void run(QueryTask* task) = 0;
};

HQC_TYPEDEF_P(QueryTaskRunner);

// ========================================================================

class QueryTaskThread;

class QueryTaskHandler
{
public:
  QueryTaskHandler(QueryTaskRunner_p handler, bool useThread = false);
  ~QueryTaskHandler();

  void postTask(QueryTask* task);
  void dropTask(QueryTask* task);

  QueryTaskRunner_p runner()
    { return mRunner; }

private:
  QueryTaskRunner_p mRunner;
  QueryTaskThread* mThread;
};

HQC_TYPEDEF_P(QueryTaskHandler);

#endif // COMMON_QUERYTASKHANDLER_HH
