
#include "AnimatedLabel.hh"

#include <QImage>

#define MILOGGER_CATEGORY "kvhqc.AnimatedLabel"
#include "util/HqcLogging.hh"
     
AnimatedLabel::AnimatedLabel(const QString &image, int imageCount, int interval, int width, QWidget *parent)
  : QLabel(parent), mFirst(0), mCount(1), mCurrent(0)
{
  METLIBS_LOG_SCOPE();
  QImage img(image);
  const int subImageHeight = img.height() / imageCount;
  METLIBS_LOG_DEBUG(LOGVAL(img.width()) << LOGVAL(img.height()) << LOGVAL(imageCount));
  for (int i = 0; i < imageCount; ++i) {
    QImage si = img.copy(0, i * subImageHeight, img.width(), subImageHeight);
    QPixmap sp(si);
    if (width > 0)
      sp = sp.scaledToWidth(width);
    mPixmaps.push_back(sp);
  }
  
  connect(&timer, SIGNAL(timeout()), this, SLOT(changeImage()));
  timer.setInterval(interval);
}

void AnimatedLabel::fix(int subimage)
{
  METLIBS_LOG_SCOPE(LOGVAL(subimage));
  timer.stop();
  mFirst = mCurrent = subimage;
  mCount = 1;
  setImage();
}

void AnimatedLabel::animate(int first, int last)
{
  METLIBS_LOG_SCOPE(LOGVAL(first) << LOGVAL(last));
  mFirst = mCurrent = first;
  mCount = last + 1 - mFirst;
  setImage();
  timer.start();
}

void AnimatedLabel::changeImage()
{
  if (mCount > 1)
    mCurrent = ((mCurrent+1 - mFirst) % mCount) + mFirst;
  setImage();
}

void AnimatedLabel::setImage()
{
  if (mCurrent >= 0 and mCurrent < mPixmaps.size())
    setPixmap(mPixmaps.at(mCurrent));
  else
    setText("?");
}
