
#ifndef COMMON_HQCUSERCONFIG_HH
#define COMMON_HQCUSERCONFIG_HH 1

#include <QtCore/QMap>
#include <QtGui/QBrush>

class HqcUserConfig {
public:
  HqcUserConfig();
  ~HqcUserConfig();

  virtual QColor dataOrigUI2Background(int ui_2);
  virtual void setDataOrigUI2Background(int ui_2, const QColor& color);

  virtual QColor dataAggregatedBackground();
  virtual void setDataAggregatedBackground(const QColor& color);

private:
  void readSettings();

private:
  typedef QMap<int,QColor> DataOriginalUI2Colors_t;
  DataOriginalUI2Colors_t mDataOrigUI2Colors;
  QColor mDataAggregatedColor;
};

#endif // COMMON_HQCUSERCONFIG_HH
