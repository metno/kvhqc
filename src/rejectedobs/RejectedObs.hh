// -*- c++ -*-

#ifndef HQC_REJECTEDOBS_HH
#define HQC_REJECTEDOBS_HH

#include "common/KvTypedefs.hh"
#include "common/TimeSpan.hh"

#include <QtGui/QDialog>
#include <QtCore/QAbstractTableModel>

class BusyLabel;
class QTableView;
class QueryTaskHelper;

class RejectDecodeTableModel : public QAbstractTableModel
{ Q_OBJECT;
public:
  RejectDecodeTableModel(const hqc::kvRejectdecode_v& rejected);
  virtual ~RejectDecodeTableModel();

  virtual int rowCount(const QModelIndex&) const;
  virtual int columnCount(const QModelIndex&) const;
  virtual QVariant data(const QModelIndex& index, int role) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
  hqc::kvRejectdecode_v mRecjected;
  QRegExp mDataRegexp;
};

// ========================================================================

class Rejects : public QDialog
{ Q_OBJECT;
public:
  Rejects(const TimeSpan& time, QWidget* parent=0);
  ~Rejects();

  static void showRejected(QWidget* parent);

protected:
  virtual void changeEvent(QEvent *event);

private:
  void retranslateUi();
  void readSettings();
  void writeSettings();

private Q_SLOTS:
  void onQueryDone();

private:
  QTableView* mTableView;
  BusyLabel* mBusy;

  RejectDecodeTableModel* mTableModel;
  QueryTaskHelper* mTask;
};

#endif // HQC_REJECTEDOBS_HH
