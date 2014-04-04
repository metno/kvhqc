
#ifndef ACCESS_SYNCREQUESTPRIVATE_HH
#define ACCESS_SYNCREQUESTPRIVATE_HH 1

#include "ObsAccess.hh"
#include "SignalRequest.hh"

#include <QtCore/QObject>
#include <QtCore/QSemaphore>

class ExecSyncRequest : public QObject
{ Q_OBJECT;
public:
  ExecSyncRequest(ObsRequest_p request);
  
  SignalRequest_p exec(ObsAccess_p access);

private Q_SLOTS:
  void onCompleted(bool);

private:
  SignalRequest_p mRequest;
  QSemaphore semaphore;
};

#endif // ACCESS_SYNCREQUESTPRIVATE_HH
