
#include "BusyLabel.hh"

#include <QImage>

static const char BUSY_ANIMATION[] = "icons:busy_animation.svg";
     
BusyLabel::BusyLabel(QWidget *parent)
  : AnimatedLabel(BUSY_ANIMATION, 5, 200, 24, parent)
{
  setFixedSize(QSize(24,24));
  setBusy(false);
}

void BusyLabel::setBusy(bool busy)
{
  if (not busy)
    fix(0);
  else
    animate(1, 4);
}
