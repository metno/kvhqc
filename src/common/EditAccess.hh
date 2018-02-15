
#ifndef EditAccess_hh
#define EditAccess_hh 1

#include "KvalobsData.hh"
#include "KvalobsUpdate.hh"
#include "ObsAccess.hh"
#include <QObject>

#include <memory>
class EditAccessPrivate;

/*! Access to editable data. Supports undo and redo.
 */
class EditAccess : public QObject, public ObsAccess
{ Q_OBJECT;

public:
  /*! Edit data from \c backend */
  EditAccess(ObsAccess_p backend);
  ~EditAccess();

  void postRequest(ObsRequest_p request) Q_DECL_OVERRIDE;
  void dropRequest(ObsRequest_p request) Q_DECL_OVERRIDE;

  ObsUpdate_p createUpdate(ObsData_p data) Q_DECL_OVERRIDE;
  ObsUpdate_p createUpdate(const SensorTime& sensorTime) Q_DECL_OVERRIDE;
  bool storeUpdates(const ObsUpdate_pv& updates) Q_DECL_OVERRIDE;

  virtual bool storeToBackend();

  void newVersion();
  void undoVersion();
  void redoVersion();
  bool canUndo() const;
  bool canRedo() const;
  size_t highestVersion() const;
  size_t currentVersion() const;
  size_t countU() const;
  const timeutil::ptime& versionTimestamp(size_t version) const;
  ObsData_pv versionChanges(size_t version) const;
  void reset();

Q_SIGNALS:
  void currentVersionChanged(size_t current, size_t highest);

protected:
  virtual KvalobsData_p createDataForUpdate(KvalobsUpdate_p update, const timeutil::ptime& tbtime);
  virtual bool fillBackendupdate(ObsUpdate_p backendUpdate, ObsData_p currentData);

private:
  //void sendObsDataChanged(ObsDataChange what, ObsDataPtr obs, int dUpdated, int dTasks);
  //void onBackendDataChanged(ObsAccess::ObsDataChange what, ObsDataPtr obs);
  void updateToCurrentVersion(bool drop);
  //void addEditTimes(TimeSet& times, const std::vector<Sensor>& sensors, const TimeSpan& limits);

private:
  std::unique_ptr<EditAccessPrivate> p;
};

HQC_TYPEDEF_P(EditAccess);

#endif // EditAccess_hh
