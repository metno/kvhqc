
#ifndef OBSCOLUMN_HH
#define OBSCOLUMN_HH 1

#include "access/ObsAccess.hh"
#include "util/timeutil.hh"
#include <QtCore/QAbstractTableModel>
#include <QtCore/QObject>
#include <boost/shared_ptr.hpp>

class ObsColumn;
HQC_TYPEDEF_P(ObsColumn);

class ObsColumn : public QObject, public boost::enable_shared_from_this<ObsColumn> {
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
  
  virtual Qt::ItemFlags flags(const timeutil::ptime& time) const;
  virtual QVariant data(const timeutil::ptime& time, int role) const = 0;
  virtual bool setData(const timeutil::ptime& time, const QVariant& value, int role);
  virtual QVariant headerData(Qt::Orientation orientation, int role) const = 0;
  
  virtual const boost::posix_time::time_duration& timeOffset() const = 0;
  virtual Sensor sensor() const;
  virtual int type() const = 0;
  
  //boost::signal2<void, timeutil::ptime, ObsColumn_p> columnChanged;
  
protected:
  boost::posix_time::time_duration mTimeOffset;
};

#endif // OBSCOLUMN_HH
