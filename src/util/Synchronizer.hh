
#ifndef SYNCHRONIZER_HH
#define SYNCHRONIZER_HH 1

#include <QObject>
#include <QSemaphore>

class Synchronizer : public QObject
{ Q_OBJECT;
public:
  bool waitForSignal();
  static bool acquireWithSignalProcessing(QSemaphore& semaphore);

public Q_SLOTS:
  void taskDone();

private:
  QSemaphore semaphore;
};

#endif // SYNCHRONIZER_HH
