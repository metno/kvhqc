
#ifndef OBSCOLUMN_HH
#define OBSCOLUMN_HH 1

#include "ObsAccess.hh"
#include "QueryTask.hh"
#include "util/timeutil.hh"
#include <QAbstractTableModel>
#include <QObject>
#include <memory>

class ObsColumn;
HQC_TYPEDEF_P(ObsColumn);
HQC_TYPEDEF_PV(ObsColumn);

class ObsTableModel;

class ObsColumn : public QObject, public std::enable_shared_from_this<ObsColumn>
{ Q_OBJECT;
public:
  enum ValueType { Numerical=1, TextCode=2, Text=4 };
  enum { ValueTypeRole = Qt::UserRole, TextCodesRole, TextCodeExplanationsRole };
  
  enum Type { ORIGINAL,
              NEW_CORRECTED,
              NEW_CONTROLINFO,
              MODEL,
              N_DISPLAYTYPES };
  
  ObsColumn() { }
  virtual ~ObsColumn() { }

  virtual void attach(ObsTableModel* table);
  virtual void detach(ObsTableModel* table);
  
  virtual Qt::ItemFlags flags(const timeutil::ptime& time) const;
  virtual QVariant data(const timeutil::ptime& time, int role) const = 0;
  virtual bool setData(const timeutil::ptime& time, const QVariant& value, int role);
  virtual QVariant headerData(Qt::Orientation orientation, int role) const = 0;
  
  virtual const boost::posix_time::time_duration& timeOffset() const = 0;
  virtual const Sensor& sensor() const;
  virtual Time_s times() const
    { return Time_s(); }
  virtual int type() const = 0;

  virtual bool isBusy() const
    { return false; }

Q_SIGNALS:
  void columnChanged(const timeutil::ptime& time, ObsColumn_p column);
  void columnTimesChanged(ObsColumn_p column);
  void columnBusyStatus(bool busy);
  
protected:
  boost::posix_time::time_duration mTimeOffset;
};

#endif // OBSCOLUMN_HH
