
#ifndef HQC_USERSETTINGS_HH
#define HQC_USERSETTINGS_HH 1

#include <QtGui/QDialog>
#include <QtGui/QLabel>
#include <memory>

namespace Ui {
class UserSettings;
}

class UserSettings : public QDialog
{ Q_OBJECT;
public:
  UserSettings(QWidget* parent=0);
  ~UserSettings();

public Q_SLOTS:
  virtual void accept();

private Q_SLOTS:
  void onOriginalColorUI2_1();
  void onOriginalColorUI2_2();
  void onOriginalColorUI2_3();
  void onOriginalColorUI2_9();
  void onAggregatedColor();

private:
  void initDataColors();
  void acceptDataColors();
  void onOriginalColorUI2(QLabel* label, QColor& colorUI2);

  void initLanguage();
  void acceptLanguage();

private:
  std::auto_ptr<Ui::UserSettings> ui;
  QColor mColorUI2_1, mColorUI2_2, mColorUI2_3, mColorUI2_9;
  QColor  mColorAggregated;
};

#endif // HQC_USERSETTINGS_HH
