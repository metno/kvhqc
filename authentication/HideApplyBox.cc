
#include "HideApplyBox.hh"

HideApplyBox::HideApplyBox(QWidget* parent)
    : QWidget(parent)
{
    setupUi(this);

    connect(hideButton, SIGNAL(clicked()), this, SIGNAL(hide()));
    connect(applyButton, SIGNAL(clicked()), this, SIGNAL(apply()));

    connect(applyHideButton, SIGNAL(clicked()), this, SLOT(hideApply()));
}

void HideApplyBox::hideApply()
{
    emit apply();
    emit hide();
}
