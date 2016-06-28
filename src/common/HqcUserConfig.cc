
#include "HqcUserConfig.hh"

#include <QtCore/QSettings>

#define MILOGGER_CATEGORY "kvhqc.HqcUserConfig"
#include "util/HqcLogging.hh"

namespace {

const char SETTING_DATA_ORIGINAL_BACKGROUND[] = "data_original_background_%1";

QString key_DATA_ORIGINAL_BACKGROUND(int ui_2)
{ return QString(SETTING_DATA_ORIGINAL_BACKGROUND).arg(ui_2); }

const QString key_DATA_AGGREGATED_BACKGROUND = "data_aggregated_background";

bool readColorFromSettings(const QSettings& settings, const QString& key, QColor& color)
{
  if (!settings.contains(key))
    return false;
  const QColor c = settings.value(key).value<QColor>();
  if (!c.isValid())
    return false;
  color = c;
  return true;
}

} // anonymous namespace

HqcUserConfig::HqcUserConfig()
{
  readSettings();
}

HqcUserConfig::~HqcUserConfig()
{
}

QColor HqcUserConfig::dataOrigUI2Background(int ui_2)
{
  return mDataOrigUI2Colors.value(ui_2);
}

void HqcUserConfig::readSettings()
{
  METLIBS_LOG_SCOPE();
  QSettings settings;

  mDataAggregatedColor = QColor(0xEE, 0xEE, 0x88);
  readColorFromSettings(settings, key_DATA_AGGREGATED_BACKGROUND, mDataAggregatedColor);

  mDataOrigUI2Colors.insert(1, QColor(0xFF, 0xFF, 0xF0)); // probably ok,     very light yellow
  mDataOrigUI2Colors.insert(2, QColor(0xFF, 0xF0, 0xF0)); // probably wrong,  very light rose
  mDataOrigUI2Colors.insert(3, QColor(0xFF, 0xE0, 0xE0)); // wrong,           light rose
  mDataOrigUI2Colors.insert(9, QColor(0xFF, 0xE0, 0xB0)); // no quality info, light orange/flesh
  for (int i=0; i<10; ++i) {
    QColor bg;
    if (readColorFromSettings(settings, key_DATA_ORIGINAL_BACKGROUND(i), bg))
      mDataOrigUI2Colors.insert(i, bg);
  }
}

void HqcUserConfig::setDataOrigUI2Background(int ui_2, const QColor& color)
{
  METLIBS_LOG_SCOPE();
  if (not color.isValid())
    return;

  mDataOrigUI2Colors.insert(ui_2, color);

  QSettings settings;
  settings.setValue(key_DATA_ORIGINAL_BACKGROUND(ui_2), color);
}

QColor HqcUserConfig::dataAggregatedBackground()
{
  return mDataAggregatedColor;
}

void HqcUserConfig::setDataAggregatedBackground(const QColor& color)
{
  if (not color.isValid())
    return;

  mDataAggregatedColor = color;

  QSettings settings;
  settings.setValue(key_DATA_AGGREGATED_BACKGROUND, color);
}
