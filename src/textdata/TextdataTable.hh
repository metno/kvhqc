// -*- c++ -*-

#ifndef HQC_TEXTDATATABLE_HH
#define HQC_TEXTDATATABLE_HH

#include "TxtDat.hh"
#include "common/TimeRange.hh"

#include <QtGui/QDialog>
#include <QtCore/QAbstractTableModel>

class TextDataTableModel : public QAbstractTableModel
{ Q_OBJECT;
public:
  TextDataTableModel(const std::vector<TxtDat>& txtList);
  virtual ~TextDataTableModel();

  virtual int rowCount(const QModelIndex&) const;
  virtual int columnCount(const QModelIndex&) const;
  virtual QVariant data(const QModelIndex& index, int role) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
private:
  std::vector<TxtDat> mTxtList;
};

// ========================================================================

class TextData : public QDialog
{ Q_OBJECT;
public:
  TextData(const std::vector<TxtDat>&, QWidget* parent=0);

  static void showTextData(int stationId, const TimeRange& timeLimits, QWidget* parent);

protected:
  virtual void changeEvent(QEvent *event);

private:
  void retranslateUi();

private:
  std::auto_ptr<TextDataTableModel> mTableModel;
};

#endif // HQC_TEXTDATATABLE_HH
