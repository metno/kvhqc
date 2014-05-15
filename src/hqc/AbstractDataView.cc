
#include "AbstractDataView.hh"

#define MILOGGER_CATEGORY "kvhqc.AbstractDataView"
#include "common/ObsLogging.hh"

AbstractDataView::AbstractDataView(QWidget* parent)
  : VisibleWidget(parent)
{
  connect(this, SIGNAL(visibilityUpdate(bool)),
      this, SLOT(onVisibilityUpdate(bool)));
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
