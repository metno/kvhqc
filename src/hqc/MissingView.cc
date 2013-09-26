/*
 * MissingView.cc
 *
 *  Created on: Sep 26, 2013
 *      Author: vegardb
 */

#include "MissingView.hh"
#include <QDateEdit>
#incldue <QDate>


MissingView::MissingView(QWidget* parent) :
    QWidget(parent)
{
  QDate today = QDate::currentDate();
  to_ = new QDateEdit(today, this);
  from_ = new QDateEdit(today.addDays(-3), this);
}

MissingView::~MissingView()
{
}

//const TaskSpecification * TaskSpecificationSelector::currentTaskSpecification() const
//{
//  QDate f = fromDateEdit->date();
//  QDate t = toDateEdit->date();
//  std::string typeString = typeID->currentText().toStdString();
//  int type = internal::typeID.find( typeString.c_str() )->second;
//  // int type = typeID->currentText().toInt(); // a field containing text yields 0
//
//  TaskSpecification * ret = new TaskSpecification(
//                boost::gregorian::date(f.year(), f.month(), f.day()),
//                boost::gregorian::date(t.year(), t.month(), t.day()),
//                type );
//  return ret;
//}


void TaskSpecificationSelector::dateCheck()
{
        struct SignalBlocker
        {
                QObject * w;
                SignalBlocker(QWidget * w) :
                        w(w)
                {
                        w->blockSignals(true);
                }
                ~SignalBlocker()
                {
                        w->blockSignals(false);
                }
        };

        QDate from = fromDateEdit->date();
        QDate to = toDateEdit->date();

        if (from > to)
        {
                if (toDateEdit->hasFocus())
                {
                        SignalBlocker sb(toDateEdit);
                        toDateEdit->setDate(from);
                }
                else
                {
                        SignalBlocker sb(fromDateEdit);
                        fromDateEdit->setDate(to);
                }
        }
}


