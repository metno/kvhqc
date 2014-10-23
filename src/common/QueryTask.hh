
#ifndef QUERYTASK_HH
#define QUERYTASK_HH 1

#include "util/boostutil.hh"

#include <QMutex>
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

/*! SQL query to be processed via QueryTaskHandler.
 *
 * It is actually less basic than could be possible: only querySql,
 * notifyRow, and notifyDone are used by QueryTaskHandler. The rest is
 * convenience and could be moved to subclasses.
 *
 * Plans: add sequence number to querySql, notifyRow, notifyDone?
 */
class BasicSQLTask : public QObject
{ Q_OBJECT;
public:
  BasicSQLTask();
  virtual ~BasicSQLTask();
  
  virtual QString querySql(QString dbversion) const = 0;

  virtual void notifyRow(const ResultRow& row) = 0;

  /*! Called when handler finished processing the task.
   *
   * This will not be called when QueryTaskHandler::dropTask returned
   * true.
   *
   * This is the last call from the task handler on this object, and
   * after this call the handler will not hold a reference to the task
   * any more.
   */
  virtual void notifyDone(const QString& withError);

  void deleteWhenDone();

  /*! \return the "amount of work remaining".
   */
  virtual int remaining()
    { return 1; }

Q_SIGNALS:
  void taskDone(const QString& withError);
  void taskRemaining(int before, int now);

private:
  QMutex mMutex;
  bool mDone, mDeleteWhenDone;
};

HQC_TYPEDEF_X(BasicSQLTask);

// ========================================================================

/*! Observation data request. */
class QueryTask : public BasicSQLTask
{
public:
  QueryTask(size_t priority)
    : mPriority(priority) { }
  
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
