/*
 * MissingList.hh
 *
 *  Created on: Sep 26, 2013
 *      Author: vegardb
 */

#ifndef MISSINGLIST_HH_
#define MISSINGLIST_HH_

#include <QWidget>
#include "internal/MissingList.hh"


class QTreeView;


class MissingView : public QWidget
{
  Q_OBJECT
public:
  MissingView(QWidget* parent = 0);

  virtual ~MissingView();


  QTreeView * getView() { return view; }

private:
  QTreeView * view;
};

#endif /* MISSINGLIST_HH_ */
