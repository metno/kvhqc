
#include "AbstractDataView.hh"

#define MILOGGER_CATEGORY "kvhqc.AbstractDataView"
#include "common/ObsLogging.hh"

AbstractDataView::AbstractDataView(QWidget* parent)
  : VisibleWidget(parent)
{
  connect(this, &AbstractDataView::visibilityUpdate, this, &AbstractDataView::onVisibilityUpdate);
  mNavigate.updateVisible(visibility());
}

AbstractDataView::~AbstractDataView()
{
}

void AbstractDataView::onVisibilityUpdate(bool visible)
{
  if (mNavigate.updateVisible(visible))
    doNavigateTo();
}

void AbstractDataView::navigateTo(const SensorTime& st)
{
  METLIBS_LOG_SCOPE(LOGMYTYPE() << LOGVAL(st));
  NavigateHelper::Blocker block(mNavigate);
  if (mNavigate.go(st))
    doNavigateTo();
}

void AbstractDataView::sendNavigateTo(const SensorTime& st)
{
  METLIBS_LOG_SCOPE(LOGVAL(st));
  NavigateHelper::Blocker block(mNavigate);
  Q_EMIT signalNavigateTo(st);
}
