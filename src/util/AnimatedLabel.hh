
#ifndef ANIMATED_LABEL_HH
#define ANIMATED_LABEL_HH 1

#include <QString>
#include <QTimer>
#include <QLabel>
#include <QList>
#include <QPixmap>

class AnimatedLabel : public QLabel
{
  Q_OBJECT;
     
public:
  AnimatedLabel(const QString & image, int imageCount, int interval/* ms */, int width=0, QWidget* parent = 0);
  
  void fix(int subimage);
  void animate(int subimage0, int subimage1);

private Q_SLOTS:
  void changeImage();

private:
  void setImage();
  
private:
  QList<QPixmap> mPixmaps;
  int mFirst, mCount, mCurrent;
  QTimer timer;
};
     
#endif // ANIMATED_LABEL_HH
