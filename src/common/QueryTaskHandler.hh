
#ifndef COMMON_QUERYTASKHANDLER_HH
#define COMMON_QUERYTASKHANDLER_HH 1

#include "util/boostutil.hh"

class QueryTask;

class QueryTaskRunner
{
public:
  virtual ~QueryTaskRunner();

  /*! Initialize the task runner. Called from the task handler thread.
   */
  virtual void initialize() = 0;

  /*! Initialize the task runner. Called from the task handler thread.
   */
  virtual void finalize() = 0;

  /*!
   * Process the task and call notifyRow and notifyStatus
   * underway. Does not call notifyDone (this is done by the
   * QueryTaskHandler).
   *
   * Called from the task handler thread.
   */
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

  /*! Enqueue a task.
   * 
   * Callbacks will come from a different thread.
   * 
   * The task may not be deleted before either notifyDone was called
   * or dropTask returned true.
   */
  void postTask(QueryTask* task);

  /*! Unqueue a task.
   * 
   * Returns true if the task has been removed from the queue before
   * processing started, and false if task processing has started (it
   * may have finished already) or if the task was never enqueued (the
   * latter will probably cause misbehaviour because notifyDone will
   * never be called).
   */
  bool dropTask(QueryTask* task);

  QueryTaskRunner_p runner()
    { return mRunner; }

private:
  QueryTaskRunner_p mRunner;
  QueryTaskThread* mThread;
};

HQC_TYPEDEF_P(QueryTaskHandler);
HQC_TYPEDEF_X(QueryTaskHandler);

#endif // COMMON_QUERYTASKHANDLER_HH
