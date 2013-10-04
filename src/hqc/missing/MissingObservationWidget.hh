#ifndef MISSINGOBSERVATIONWIDGET_H
#define MISSINGOBSERVATIONWIDGET_H

#include <QWidget>
#include <boost/signal.hpp>

class MissingSelectorWidget;
class MissingView;
class QDate;
class SensorTime;


class MissingObservationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MissingObservationWidget(QWidget *parent = 0);

    boost::signal1<void, SensorTime> signalNavigateTo;

public Q_SLOTS:
    void findMissing();
    void findMissing(const QDate & from, const QDate & to, int type);

//private Q_SLOTS:
    void signalNavigate(const SensorTime & st);

private:
    MissingSelectorWidget * selector;
    MissingView * view;
};

#endif // MISSINGOBSERVATIONWIDGET_H
