// -*- c++ -*-

#ifndef HQC_REJECTEDOBS_HH
#define HQC_REJECTEDOBS_HH

#include <QtGui/QDialog>
#include <QtCore/QAbstractTableModel>
#include <QtCore/QRegExp>
#include <memory>
#include <vector>

namespace kvalobs {
class kvRejectdecode;
}

class RejectDecodeTableModel : public QAbstractTableModel
{ Q_OBJECT;
public:
  RejectDecodeTableModel(const std::vector<kvalobs::kvRejectdecode>& txtList);
  virtual ~RejectDecodeTableModel();

  virtual int rowCount(const QModelIndex&) const;
  virtual int columnCount(const QModelIndex&) const;
  virtual QVariant data(const QModelIndex& index, int role) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
  std::vector<kvalobs::kvRejectdecode> mRecjectList;
  QRegExp mDataRegexp;
};

// ========================================================================

class Rejects : public QDialog
{ Q_OBJECT;
public:
  Rejects(const std::vector<kvalobs::kvRejectdecode>&, QWidget* parent=0);

protected:
  virtual void changeEvent(QEvent *event);

private:
  void retranslateUi();

private:
  std::auto_ptr<RejectDecodeTableModel> mTableModel;
};

#endif // HQC_REJECTEDOBS_HH
