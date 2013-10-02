

#include "MissingObservationWidget.hh"
#include "MissingSelectorWidget.hh"
#include "MissingView.hh"
#include "missingdatamodel.hh"
#include "internal/findMissingStations.hh"
#include "internal/MissingList.hh"
#include "internal/TaskSpecification.hh"
#include <common/KvMetaDataBuffer.hh>
#include <QBoxLayout>
#include <QTreeView>
#include <QDebug>
#include <boost/date_time/gregorian_calendar.hpp>
#include <algorithm>


MissingObservationWidget::MissingObservationWidget(QWidget *parent) :
    QWidget(parent)
{
    selector = new MissingSelectorWidget(this);
    view = new MissingView(this);

    connect(selector, SIGNAL(findMissingRequested()), SLOT(findMissing()));

    QVBoxLayout * mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(selector);
    mainLayout->addWidget(view);
}

void MissingObservationWidget::findMissing()
{
    findMissing(selector->from(), selector->to(), selector->type());
}


void MissingObservationWidget::findMissing(const QDate & from, const QDate & to, int type)
{
    MissingList missing;

    using boost::gregorian::date;

    TaskSpecification spec(date(from.year(), from.month(), from.day()),
                           date(to.year(), to.month(), to.day()),
                           type);

    bool stop = false;
    findMissingStations( missing, spec, stop );

    MissingDataModel * model = new MissingDataModel(missing, this);
    view->getView()->setModel(model);
}
