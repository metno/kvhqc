
#ifndef util_VectorModel_hh
#define util_VectorModel_hh 1

#include <QtCore/QAbstractListModel>
#include <vector>

namespace VectorModelDetail {

template<class ValueType>
struct BasicExtract {
  typedef ValueType value_t;
};

template<class Extract>
class OverrideExtract {
public:
  typedef typename Extract::value_t value_t;

  OverrideExtract(const Extract& e = Extract())
    : mExtract(e) { }

  QVariant text(const value_t& v) const;
  QVariant tip(const value_t& v) const;

  void override(const value_t& value, const QString& tooltip, const QString& label="")
    { mOverrides[value] = OverrideData_t(label.isEmpty() ? tooltip : label, tooltip); }

private:
  typedef std::pair<QString,QString> OverrideData_t;
  typedef std::map<value_t, OverrideData_t> Overrides_t;
  Extract mExtract;
  Overrides_t mOverrides;
};

template<class Extract>
QVariant OverrideExtract<Extract>::text(const value_t& v) const
{
  typename Overrides_t::const_iterator it = mOverrides.find(v);
  if (it != mOverrides.end())
    return it->second.first;
  return mExtract.text(v);
}

template<class Extract>
QVariant OverrideExtract<Extract>::tip(const value_t& v) const
{
  typename Overrides_t::const_iterator it = mOverrides.find(v);
  if (it != mOverrides.end())
    return it->second.second;
  return mExtract.tip(v);
}

} // namespace VectorModelDetail

// ========================================================================

template<class Extract>
class VectorModel : public QAbstractListModel {
public:
  typedef typename Extract::value_t value_t;
  typedef std::vector<value_t> vector_t;

  VectorModel(const vector_t& values, const Extract& e = Extract())
    : mValues(values), mExtract(e) { }

  int rowCount(const QModelIndex&) const
    { return mValues.size(); }

  QVariant data(const QModelIndex& index, int role) const;
  
  const vector_t& values() const
    { return mValues; }

private:
  vector_t mValues;
  Extract mExtract;
};

template<class Extract>
QVariant VectorModel<Extract>::data(const QModelIndex& index, int role) const
{
  if (role == Qt::DisplayRole)
    return mExtract.text(mValues.at(index.row()));
  else if (role == Qt::ToolTipRole or role == Qt::StatusTipRole)
    return mExtract.tip(mValues.at(index.row()));
  else
    return QVariant();
}

#endif // util_VectorModel_hh
