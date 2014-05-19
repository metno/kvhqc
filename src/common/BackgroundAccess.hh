
#ifndef ACCESS_BACKGROUNDACCESS_HH
#define ACCESS_BACKGROUNDACCESS_HH 1

#include "ObsAccess.hh"
#include <QtCore/QSemaphore>
#include <string>

class QueryTask;

class BackgroundHandler : public QObject
{ Q_OBJECT;
public:
  virtual ~BackgroundHandler();
  virtual void initialize() = 0;
  virtual void finalize() = 0;
  virtual void queryTask(QueryTask* task) = 0;
};

HQC_TYPEDEF_P(BackgroundHandler);

// ========================================================================

class BackgroundThread;

class BackgroundAccess : public QObject, public ObsAccess
{ Q_OBJECT;
protected:
  BackgroundAccess(BackgroundHandler_p handler, bool useThread = false);

public:
  ~BackgroundAccess();

  virtual void postRequest(ObsRequest_p request);
  virtual void dropRequest(ObsRequest_p request);

protected:
  BackgroundHandler_p handler()
    { return mHandler; }

  const ObsRequest_pv& requests() const
    { return mRequests; }

  void distributeUpdates(const ObsData_pv& updated, const ObsData_pv& inserted, const SensorTime_v& dropped);
  QueryTask* taskForRequest(ObsRequest_p request);

private Q_SLOTS:
  void onNewData(ObsRequest_p request, const ObsData_pv& data);

private:
  BackgroundHandler_p mHandler;

  BackgroundThread* mThread;

  ObsRequest_pv mRequests;
};

HQC_TYPEDEF_P(BackgroundAccess);

#endif // ACCESS_BACKGROUNDACCESS_HH
