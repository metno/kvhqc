
#include "AbstractDataView.hh"

#include <QtGui/QResizeEvent>

#define MILOGGER_CATEGORY "kvhqc.AbstractDataView"
#include "common/ObsLogging.hh"

AbstractDataView::AbstractDataView(QWidget* parent)
  : QWidget(parent)
{
}

AbstractDataView::~AbstractDataView()
{
}

void AbstractDataView::showEvent(QShowEvent* se)
{
  QWidget::showEvent(se);
  if (mNavigate.updateVisible(true))
    doNavigateTo();
}

void AbstractDataView::hideEvent(QHideEvent* he)
{
  QWidget::hideEvent(he);
  mNavigate.updateVisible(false);
}

void AbstractDataView::resizeEvent(QResizeEvent *re)
{
  QWidget::resizeEvent(re);
  if (mNavigate.updateVisible(not re->size().isEmpty()))
    doNavigateTo();
}

void AbstractDataView::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::LanguageChange)
    retranslateUi();
  QWidget::changeEvent(event);
}

void AbstractDataView::retranslateUi()
{
}

void AbstractDataView::navigateTo(const SensorTime& st)
{
  if (mNavigate.go(st))
    doNavigateTo();
}

void AbstractDataView::sendNavigateTo(const SensorTime& st)
{
  METLIBS_LOG_SCOPE(LOGVAL(st));
  NavigateHelper::Blocker block(mNavigate);
  if (mNavigate.go(st))
    Q_EMIT signalNavigateTo(st);
}
