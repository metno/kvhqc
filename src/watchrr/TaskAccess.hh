
#ifndef TaskAccess_hh
#define TaskAccess_hh 1

#include "common/EditAccess.hh"

class TaskAccess : public EditAccess
{
public:
  /*! Task data from \c backend */
  TaskAccess(ObsAccess_p backend);
  ~TaskAccess();

  ObsUpdate_p createUpdate(ObsData_p data);
  ObsUpdate_p createUpdate(const SensorTime& sensorTime);

  bool storeToBackend();

  ObsData_p findE(const SensorTime& st);

protected:
  KvalobsData_p createDataForUpdate(KvalobsUpdate_p update, const timeutil::ptime& tbtime);
};

HQC_TYPEDEF_P(TaskAccess);

#endif // TaskAccess_hh
