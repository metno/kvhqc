
#include "VisibleWidget.hh"

#include <QtGui/QResizeEvent>

VisibleWidget::VisibleWidget(QWidget* parent)
  : QWidget(parent)
{
}

VisibleWidget::~VisibleWidget()
{
}

void VisibleWidget::showEvent(QShowEvent* se)
{
  QWidget::showEvent(se);
  Q_EMIT visibilityUpdate(true);
}

void VisibleWidget::hideEvent(QHideEvent* he)
{
  QWidget::hideEvent(he);
  Q_EMIT visibilityUpdate(false);
}

void VisibleWidget::resizeEvent(QResizeEvent *re)
{
  QWidget::resizeEvent(re);
  Q_EMIT visibilityUpdate(not re->size().isEmpty());
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
