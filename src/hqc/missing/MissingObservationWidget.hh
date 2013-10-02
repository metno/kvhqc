#ifndef MISSINGOBSERVATIONWIDGET_H
#define MISSINGOBSERVATIONWIDGET_H

#include <QWidget>

class MissingSelectorWidget;
class MissingView;
class QDate;


class MissingObservationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MissingObservationWidget(QWidget *parent = 0);
    
Q_SIGNALS:
    
public Q_SLOTS:
    void findMissing();
    void findMissing(const QDate & from, const QDate & to, int type);

private:
    MissingSelectorWidget * selector;
    MissingView * view;
};

#endif // MISSINGOBSERVATIONWIDGET_H
