
#include "HqcUserConfig.hh"

#include <QSettings>

#define MILOGGER_CATEGORY "kvhqc.HqcUserConfig"
#include "util/HqcLogging.hh"

namespace {

const char SETTING_DATA_ORIGINAL_BACKGROUND[] = "data_original_background_%1";

QString key_DATA_ORIGINAL_BACKGROUND(int ui_2)
{ return QString(SETTING_DATA_ORIGINAL_BACKGROUND).arg(ui_2); }

} // anonymous namespace

HqcUserConfig::HqcUserConfig()
{
  readDataOrigUI2Colors();
}

HqcUserConfig::~HqcUserConfig()
{
}

QColor HqcUserConfig::dataOrigUI2Background(int ui_2)
{
  return mDataOrigUI2Colors.value(ui_2);
}

void HqcUserConfig::readDataOrigUI2Colors()
{
  METLIBS_LOG_SCOPE();
  QSettings settings;
  mDataOrigUI2Colors.insert(1, QColor(0xFF, 0xFF, 0xF0)); // probably ok,     very light yellow
  mDataOrigUI2Colors.insert(2, QColor(0xFF, 0xF0, 0xF0)); // probably wrong,  very light rose
  mDataOrigUI2Colors.insert(3, QColor(0xFF, 0xE0, 0xE0)); // wrong,           light rose
  mDataOrigUI2Colors.insert(9, QColor(0xFF, 0xE0, 0xB0)); // no quality info, light orange/flesh
  for (int i=0; i<10; ++i) {
    const QString key = key_DATA_ORIGINAL_BACKGROUND(i);
    if (not settings.contains(key)) {
      METLIBS_LOG_DEBUG("no ui2 color setting  for '" << key << "'");
      continue;
    }
    const QColor bg = settings.value(key).value<QColor>();
    if (bg.isValid()) {
      METLIBS_LOG_DEBUG("read ui2 color for '" << key << "'");
      mDataOrigUI2Colors.insert(i, bg);
    }
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
