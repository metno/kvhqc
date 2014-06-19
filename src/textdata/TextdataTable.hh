// -*- c++ -*-

#ifndef HQC_TEXTDATATABLE_HH
#define HQC_TEXTDATATABLE_HH

#include "TxtDat.hh"
#include "common/TimeSpan.hh"

#include <QtGui/QDialog>
#include <QtCore/QAbstractTableModel>

class BusyLabel;
class QTableView;
class QueryTaskHelper;

class TextDataTableModel : public QAbstractTableModel
{ Q_OBJECT;
public:
  TextDataTableModel(const std::vector<TxtDat>& txtList, QObject* parent=0);
  ~TextDataTableModel();

  int rowCount(const QModelIndex&) const;
  int columnCount(const QModelIndex&) const;
  QVariant data(const QModelIndex& index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:
  std::vector<TxtDat> mTxtList;
};

// ========================================================================

class TextData : public QDialog
{ Q_OBJECT;
public:
  TextData(const int stationId, const TimeSpan& time, QWidget* parent=0);
  ~TextData();

  static void showTextData(QWidget* parent);

protected:
  virtual void changeEvent(QEvent *event);

private Q_SLOTS:
  void onQueryDone();

private:
  void retranslateUi();
  void readSettings();
  void writeSettings();

private:
  QTableView* mTableView;
  BusyLabel* mBusy;

  TextDataTableModel* mTableModel;
  QueryTaskHelper* mTask;
};

#endif // HQC_TEXTDATATABLE_HH
