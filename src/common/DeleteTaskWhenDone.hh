
#ifndef COMMON_DELETETASKWHENDONE_HH
#define COMMON_DELETETASKWHENDONE_HH 1

#include <QObject>

class QueryTask;

class DeleteTaskWhenDone : public QObject
{ Q_OBJECT;
public:
  DeleteTaskWhenDone(QueryTask* t);
  ~DeleteTaskWhenDone();

  QueryTask* task() const
    { return mTask; }

  void enableDelete();

private Q_SLOTS:
  void onTaskDone(const QString& withError);

private:
  void doDelete();

private:
  QueryTask* mTask;
  bool mDone, mDelete;
};

#endif // COMMON_DELETETASKWHENDONE_HH
