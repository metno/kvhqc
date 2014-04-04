
#include "ItemCheckBox.hh"

void ItemCheckBox::clicked()
{
    Q_EMIT clicked(mItem);
}
