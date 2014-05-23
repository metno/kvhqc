
#include "VisibleWidget.hh"

#include <QtGui/QResizeEvent>

VisibleWidget::VisibleWidget(QWidget* parent)
  : QWidget(parent)
  , mShown(true)
  , mEmpty(false)
{
}

VisibleWidget::~VisibleWidget()
{
}

void VisibleWidget::showEvent(QShowEvent* se)
{
  QWidget::showEvent(se);
  if (not mShown) {
    mShown = true;
    sendUpdate();
  }
}

void VisibleWidget::hideEvent(QHideEvent* he)
{
  QWidget::hideEvent(he);
  if (mShown) {
    mShown = false;
    sendUpdate();
  }
}

void VisibleWidget::resizeEvent(QResizeEvent *re)
{
  QWidget::resizeEvent(re);
  const bool empty = re->size().isEmpty();
  if (empty != mEmpty) {
    mEmpty = empty;
    sendUpdate();
  }
}

void VisibleWidget::sendUpdate()
{
  Q_EMIT visibilityUpdate(mShown and not mEmpty);
}

void VisibleWidget::changeEvent(QEvent *event)
{
  if (event->type() == QEvent::LanguageChange)
    retranslateUi();
  QWidget::changeEvent(event);
}

void VisibleWidget::retranslateUi()
{
}
