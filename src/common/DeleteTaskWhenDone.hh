
#ifndef COMMON_DELETETASKWHENDONE_HH
#define COMMON_DELETETASKWHENDONE_HH 1

#include <QObject>

class SignalTask;

class DeleteTaskWhenDone : public QObject
{ Q_OBJECT;
public:
  DeleteTaskWhenDone(SignalTask* t);
  ~DeleteTaskWhenDone();

  SignalTask* task() const
    { return mTask; }

  void enableDelete();

private Q_SLOTS:
  void onQueryDone();

private:
  void doDelete();

private:
  SignalTask* mTask;
  bool mDone, mDelete;
};

#endif // COMMON_DELETETASKWHENDONE_HH
