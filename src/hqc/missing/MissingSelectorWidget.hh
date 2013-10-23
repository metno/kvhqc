/*
 * MissingSelectorWidget.hh
 *
 *  Created on: Sep 26, 2013
 *      Author: vegardb
 */

#ifndef MISSINGSELECTORWIDGET_HH_
#define MISSINGSELECTORWIDGET_HH_

#include <QWidget>
#include <QDate>

class MiDateTimeEdit;
class QComboBox;


class MissingSelectorWidget : public QWidget
{
  Q_OBJECT;
public:
  MissingSelectorWidget(QWidget * parent = 0);

  virtual ~MissingSelectorWidget();

  QDate from() const;
  QDate to() const;
  int type() const;

Q_SIGNALS:
  void findMissingRequested();

private Q_SLOTS:
  void dateCheck();

private:
  MiDateTimeEdit * fromDateEdit;
  MiDateTimeEdit * toDateEdit;
  QComboBox * typeID;
};

#endif /* MISSINGSELECTORWIDGET_HH_ */
