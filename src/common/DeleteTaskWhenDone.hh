
#ifndef COMMON_DELETETASKWHENDONE_HH
#define COMMON_DELETETASKWHENDONE_HH 1

#include <QObject>

class SignalTask;

class DeleteTaskWhenDone : public QObject
{ Q_OBJECT;
public:
  DeleteTaskWhenDone(SignalTask* t);
  ~DeleteTaskWhenDone();

private Q_SLOTS:
  void onQueryDone();

private:
  SignalTask* mTask;
};

#endif // COMMON_DELETETASKWHENDONE_HH
