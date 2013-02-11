
#ifndef HIDEAPPLYBOX_HH
#define HIDEAPPLYBOX_HH 1

#include "ui_hideapplybox.h"

class HideApplyBox : public QWidget, private Ui_HideApplyBox
{
    Q_OBJECT;

public:
    HideApplyBox(QWidget* parent);

    void setCanApply(bool enabled);

Q_SIGNALS:
    void apply();
    void hide();

private Q_SLOTS:
    void hideApply();
};

#endif // HIDEAPPLYBOX_HH
