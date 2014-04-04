// -*- c++ -*-

#ifndef HQC_ERRORLISTMODEL_HH
#define HQC_ERRORLISTMODEL_HH

#include "common/AnalyseErrors.hh"
#include "common/EditAccess.hh"
#include "common/ModelAccess.hh"

#include <QtCore/QAbstractTableModel>

class ErrorListModel : public QAbstractItemModel
{ Q_OBJECT;
  class ErrorTreeItem;
  typedef ErrorTreeItem* ErrorTreeItem_P;

public:
  ErrorListModel(EditAccessPtr eda, ModelAccessPtr mda,
      const Errors::Sensors_t& sensors, const TimeRange& limits, bool errorsForSalen);
  ~ErrorListModel();

  enum EDIT_COLUMNS {
    COL_STATION_ID = 0,
    COL_STATION_NAME,
    COL_STATION_WMO,
    COL_OBS_TIME,
    COL_OBS_PARAM,
    COL_OBS_TYPEID,
    COL_OBS_ORIG,
    COL_OBS_CORR,
    COL_OBS_MODEL,
    COL_OBS_FLAGS,
    NCOLUMNS
  };

  virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
  virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
  QModelIndex parent(const QModelIndex &index) const;
  Qt::ItemFlags flags(const QModelIndex& index) const;
  virtual QVariant data(const QModelIndex& index, int role) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  QModelIndex findSensorTime(const SensorTime& st) const
    { return findSensorTime(st, mRootItem); }

  EditDataPtr findObs(const QModelIndex& index) const;

  void highlightStation(int stationID);

Q_SIGNALS:
  void beginDataChange();
  void endDataChange();

private:
  void onDataChanged(ObsAccess::ObsDataChange, ObsDataPtr);
  QModelIndex findSensorTime(const SensorTime& st, ErrorTreeItem_P item) const;
  ErrorTreeItem_P itemFromIndex(const QModelIndex& index) const;
  void buildTree(const Errors::Errors_t& errors);
  void removeRow(ErrorTreeItem_P parent, int row);

  void updateErrorItem(ErrorTreeItem_P item);
  void removeErrorItem(ErrorTreeItem_P item);
  void insertErrorItem(Errors::ErrorInfo ei);

  QModelIndex indexFromItem(ErrorTreeItem_P item, int column=0) const;

private:
  EditAccessPtr mDA;
  ModelAccessPtr mMA;
  Errors::Sensors_t mSensors;
  TimeRange mTimeLimits;
  ErrorTreeItem_P mRootItem;
  bool mErrorsForSalen;
  int mHighlightedStation;
  bool mBlockHighlighting;
};

#endif // HQC_ERRORLISTMODEL_HH
