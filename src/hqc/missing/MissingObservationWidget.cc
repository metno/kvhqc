

#include "MissingObservationWidget.hh"
#include "MissingSelectorWidget.hh"
#include "MissingView.hh"
#include "StationOrderedMissingDataModel.hh"
#include "internal/findMissingStations.hh"
#include "internal/MissingList.hh"
#include "internal/TaskSpecification.hh"
#include "common/KvMetaDataBuffer.hh"
#include "util/gui/BusyIndicator.hh"
#include <QBoxLayout>
#include <QTreeView>
#include <boost/date_time/gregorian_calendar.hpp>

#define MILOGGER_CATEGORY "kvhqc.missing.MissingObservationWidget"
#include "common/ObsLogging.hh"

MissingObservationWidget::MissingObservationWidget(QWidget *parent) :
    QWidget(parent)
{
    selector = new MissingSelectorWidget(this);
    view = new MissingView(this);

    connect(view, SIGNAL(selected(SensorTime)), this, SLOT(signalNavigate(SensorTime)));
    connect(selector, SIGNAL(findMissingRequested()), SLOT(findMissing()));

    QVBoxLayout * mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(2);
    mainLayout->setContentsMargins(2, 2, 2, 2);
    mainLayout->addWidget(selector);
    mainLayout->addWidget(view);
}

void MissingObservationWidget::findMissing()
{
    BusyIndicator busy;
    findMissing(selector->from(), selector->to(), selector->type());
}


void MissingObservationWidget::findMissing(const QDate & from, const QDate & to, int type)
{
    METLIBS_LOG_SCOPE();
    MissingList missing;

    using boost::gregorian::date;

    TaskSpecification spec(date(from.year(), from.month(), from.day()),
                           date(to.year(), to.month(), to.day()),
                           type);

    bool stop = false;
    findMissingStations( missing, spec, stop );
    METLIBS_LOG_DEBUG("Got data");

    StationOrderedMissingDataModel * model = new StationOrderedMissingDataModel(missing, this);
    view->setModel(model);
}

void MissingObservationWidget::signalNavigate(const SensorTime & st)
{
    METLIBS_LOG_SCOPE();
    METLIBS_LOG_DEBUG(st);
    signalNavigateTo(st);
}
