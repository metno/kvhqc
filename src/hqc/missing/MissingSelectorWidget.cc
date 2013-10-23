/*
 * MissingSelectorWidget.cc
 *
 *  Created on: Sep 26, 2013
 *      Author: vegardb
 */

#include "MissingSelectorWidget.hh"
#include "util/gui/MiDateTimeEdit.hh"
#include <QDate>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QBoxLayout>
#include <QMessageBox>
#include <boost/assign/list_of.hpp>
#include <boost/assign/list_inserter.hpp>
#include <boost/foreach.hpp>
#include <map>
#include <vector>


namespace
{
    // TODO wrap with QT_TRANSLATE_NOOP("MissingSelectorWidget", ...) and update use of "typeids_"
    const QString all = "Alle";
    const QString autoobs = "Autoobs";
    const QString sms2 = "SMS - type 2";
    const QString sms8 = "SMS - type 8";
    const QString sms12 = "SMS - type 12";
    const QString ukekort = "Ukekort";
    const QString dagbok = "Dagbok";

    typedef std::map<QString, int> TypeIDMap;
    TypeIDMap typeids_ = boost::assign::map_list_of
                ( all,             0 )
                ( autoobs,         3 )
                ( sms2,          302 )
                ( sms8,          308 )
                ( sms12,         312 )
                ( ukekort,       402 )
                ( dagbok,        412 );

    const std::vector<QString> orderedTypeID = boost::assign::list_of
        //(all)(sms2)(sms8)(sms12)(ukekort);
        (sms2);

void addEntries(QComboBox * out)
{
  BOOST_FOREACH(const QString & s, orderedTypeID)
    out->addItem(s);
}
}

MissingSelectorWidget::MissingSelectorWidget(QWidget * parent) :
    QWidget(parent)
{
  QDate today = QDate::currentDate();

  fromDateEdit = new MiDateTimeEdit(today.addDays(-3), this);
  fromDateEdit->setDisplayFormat("dd/MM yyyy");
  QLabel * fromLabel = new QLabel(tr("&From:"), this);
  fromLabel->setBuddy(fromDateEdit);

  toDateEdit = new MiDateTimeEdit(today, this);
  toDateEdit->setDisplayFormat("dd/MM yyyy");
  QLabel * toLabel = new QLabel(tr("&To:"), this);
  toLabel->setBuddy(toDateEdit);

  QObject::connect(fromDateEdit, SIGNAL(dateChanged(QDate)), this, SLOT(dateCheck()));
  QObject::connect(toDateEdit, SIGNAL(dateChanged(QDate)), this, SLOT(dateCheck()));


  typeID = new QComboBox(this);
  addEntries(typeID);
  QLabel * typeLabel = new QLabel(tr("&Type:"), this);
  typeLabel->setBuddy(typeID);


  QPushButton * ok = new QPushButton(tr("&Apply"));
  ok->setDefault(true);
  connect(ok, SIGNAL(clicked()), SIGNAL(findMissingRequested()));


  QHBoxLayout * mainLayout = new QHBoxLayout(this);
  mainLayout->setSpacing(2);
  mainLayout->setContentsMargins(2, 2, 2, 2);
  mainLayout->addWidget(fromLabel);
  mainLayout->addWidget(fromDateEdit);
  mainLayout->addWidget(toLabel);
  mainLayout->addWidget(toDateEdit);
  mainLayout->addWidget(typeLabel);
  mainLayout->addWidget(typeID);
  mainLayout->addStretch();
  mainLayout->addWidget(ok);
  mainLayout->addStretch(10);
}

MissingSelectorWidget::~MissingSelectorWidget()
{
}

QDate MissingSelectorWidget::from() const
{
    return fromDateEdit->date();
}

QDate MissingSelectorWidget::to() const
{
    return toDateEdit->date();
}

int MissingSelectorWidget::type() const
{
    const QString & type = typeID->currentText();
    TypeIDMap::const_iterator find = typeids_.find(type);
    if ( find == typeids_.end() ) {
      QMessageBox::warning(0, tr("Error"), tr("Unable to understand %1!").arg(typeID->currentText()));
        return -1;
    }
    return find->second;
}


void MissingSelectorWidget::dateCheck()
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

  if (from > to) {
      if (toDateEdit->hasFocus()) {
          SignalBlocker sb(toDateEdit);
          toDateEdit->setDate(from);
        }
      else {
          SignalBlocker sb(fromDateEdit);
          fromDateEdit->setDate(to);
        }
    }
}
