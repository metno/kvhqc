#include "ActionButton.hh"

#include <QAction>
     
ActionButton::ActionButton(QAbstractButton *button, QAction* action, QObject* parent)
  : QObject(parent)
  , mButton(button)
  , mAction(action)
{
  doConnect();
}
     
void ActionButton::setAction(QAction *action)
{
  if (mAction == action)
    return;

  doDisconnect();
  mAction = action;
  doConnect();
  updateFromAction();
}

void ActionButton::doConnect()
{
  if (mAction and mButton) {
    connect(mAction, SIGNAL(changed()), this, SLOT(updateFromAction()));
    connect(mButton, SIGNAL(clicked()), mAction, SLOT(trigger()));
  }
}
     
void ActionButton::doDisconnect()
{
  if (mAction and mButton) {
    disconnect(mAction, SIGNAL(changed()), this, SLOT(updateFromAction()));
    disconnect(mButton, SIGNAL(clicked()), mAction, SLOT(trigger()));
  }
}
     
void ActionButton::updateFromAction()
{
  mButton->setText(mAction->text());
  mButton->setStatusTip(mAction->statusTip());
  mButton->setToolTip(mAction->toolTip());
  mButton->setIcon(mAction->icon());
  mButton->setEnabled(mAction->isEnabled());
}
