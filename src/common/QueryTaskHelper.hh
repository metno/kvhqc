
#ifndef COMMON_QUERYTASKHELPER_HH
#define COMMON_QUERYTASKHELPER_HH 1

#include "DeleteTaskWhenDone.hh"
#include <QObject>
#include <boost/shared_ptr.hpp>

class SignalTask;
class QueryTaskHandler;

class QueryTaskHelper : public QObject
{ Q_OBJECT;

public:
  /*! Task helper for the given task. Takes ownership of the task
   *  object. Task object may not have a parent. */
  QueryTaskHelper(SignalTask* task);

  /*! Destruct helper and task. If the task has been posted and is not
   *  done yet, wait for done signal and then delete the task object
   *  via QObject::deleteLater.
   */
  ~QueryTaskHelper();

  /*! Post the task in the handler's queue.
   */
  void post(QueryTaskHandler* handler);

  void post(boost::shared_ptr<QueryTaskHandler> handler)
    { post(handler.get()); }

  /*! Post the task in the handler's queue, wait for completion.
   */
  void sync(QueryTaskHandler* handler);

  void sync(boost::shared_ptr<QueryTaskHandler> handler)
    { sync(handler.get()); }

  /*! Drop the task from the handler's queue. Even if the task is
   *  already being processed, the done signal will not be forwarded
   *  from this helper any more.
   */
  bool drop();

  /*! Access the task object. */
  SignalTask* task() const
    { return mDeleter->task(); }

private Q_SLOTS:
  void onQueryDone();

Q_SIGNALS:
  /*! Emitted when the task is done, but only when drop has not been
   *  called before.
   */
  void done(SignalTask* task);

private:
  QueryTaskHandler* mHandler;
  DeleteTaskWhenDone* mDeleter;
};

#endif // COMMON_QUERYTASKHELPER_HH
