/*
 * MissingList.hh
 *
 *  Created on: Sep 26, 2013
 *      Author: vegardb
 */

#ifndef MISSINGLIST_HH_
#define MISSINGLIST_HH_

#include <QWidget>

class QDateEdit;

class MissingView : public QWidget
{
  Q_OBJECT;
public:
  MissingView(QWidget* parent = 0);
  virtual
  ~MissingView();

  /**
   * Get a specification matching what the user have entered into the dialog.
   */
  //const TaskSpecification * currentTaskSpecification() const;

  /**
   * Strings, explaining various typeids
   */
  static QStringList typidElements;

private Q_SLOTS:
  void dateCheck();

private:
  QDateEdit * fromDateEdit;
  QDateEdit * toDateEdit;
  QComboBox * typeID;
};

#endif /* MISSINGLIST_HH_ */
