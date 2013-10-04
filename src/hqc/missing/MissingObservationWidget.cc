

#include "MissingObservationWidget.hh"
#include "MissingSelectorWidget.hh"
#include "MissingView.hh"
#include "StationOrderedMissingDataModel.hh"
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

    connect(view, SIGNAL(selected(SensorTime)), this, SLOT(signalNavigate(SensorTime)));
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
    qDebug() << "Got data";

    StationOrderedMissingDataModel * model = new StationOrderedMissingDataModel(missing, this);
    view->setModel(model);
}

void MissingObservationWidget::signalNavigate(const SensorTime & st)
{
    const Sensor & s = st.sensor;
    qDebug() << s.stationId <<", "<< s.paramId <<", "<< s.level <<", "<< s.sensor <<", "<< s.typeId;

    signalNavigateTo(st);
}
