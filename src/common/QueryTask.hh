
#ifndef QUERYTASK_HH
#define QUERYTASK_HH 1

#include "util/boostutil.hh"
#include <QString>
#include <QVariant>

class ResultRow
{
public:
  virtual ~ResultRow() { }
  virtual int getInt(int index) const = 0;
  virtual float getFloat(int index) const = 0;
  virtual std::string getStdString(int index) const = 0;
  virtual QString getQString(int index) const = 0;
};

// ========================================================================

/*! Observation data request. */
class QueryTask : boost::noncopyable
{
public:
  QueryTask(size_t priority);
  virtual ~QueryTask();
  
  virtual QString querySql(QString dbversion) const = 0;

  virtual void notifyRow(const ResultRow& row) = 0;

  enum {
    PENDING = -1,
    STARTED = 0,
    COMPLETE = 100,
    FAILED = 112
  };

  virtual void notifyStatus(int status) = 0;
  virtual void notifyError(QString message);

  /*! Called when handler finished processing the task.
   *
   * This will not be called when QueryTaskHandler::dropTask returned
   * true.
   *
   * This is the last call from the task handler on this object, and
   * after this call the handler will not hold a reference to the task
   * any more.
   */
  virtual void notifyDone();

  enum {
    PRIORITY_AUTOMATIC = 20,
    PRIORITY_SEARCH = 50,
    PRIORITY_INTERACTIVE = 80,
    PRIORITY_SYNC = 100
  };

  size_t priority() const
    { return mPriority; }

private:
  size_t mPriority;
};

HQC_TYPEDEF_X(QueryTask);

struct QueryTask_by_Priority : public std::binary_function<bool, const QueryTask*, const QueryTask*> {
  bool operator()(const QueryTask* qa, const QueryTask* qb) const
    { return qa->priority() < qb->priority(); }
};

#endif // QUERYTASK_HH
