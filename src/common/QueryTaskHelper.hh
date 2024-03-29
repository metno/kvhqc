
#ifndef COMMON_QUERYTASKHELPER_HH
#define COMMON_QUERYTASKHELPER_HH 1

#include "DeleteTaskWhenDone.hh"
#include <QObject>
#include <memory>

class QueryTask;
class QueryTaskHandler;

class QueryTaskHelper : public QObject
{ Q_OBJECT;

public:
  /*! Task helper for the given task. Takes ownership of the task
   *  object. Task object may not have a parent. */
  QueryTaskHelper(QueryTask* task);

  /*! Destruct helper and task. If the task has been posted and is not
   *  done yet, wait for done signal and then delete the task object
   *  via QObject::deleteLater.
   */
  ~QueryTaskHelper();

  /*! Post the task in the handler's queue.
   */
  void post(QueryTaskHandler* handler, bool synchronized=false);

  void post(std::shared_ptr<QueryTaskHandler> handler, bool synchronized=false)
    { post(handler.get(), synchronized); }

  /*! Drop the task from the handler's queue. Even if the task is
   *  already being processed, the done signal will not be forwarded
   *  from this helper any more.
   */
  bool drop();

  /*! Access the task object. */
  QueryTask* task() const
    { return mDeleter->task(); }

private Q_SLOTS:
  void onQueryDone();

Q_SIGNALS:
  /*! Emitted when the task is done, but only when drop has not been
   *  called before.
   */
  void done(QueryTask* task);

private:
  QueryTaskHandler* mHandler;
  DeleteTaskWhenDone* mDeleter;
};

#endif // COMMON_QUERYTASKHELPER_HH
