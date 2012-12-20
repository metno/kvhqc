
#include "ObsDelegate.hh"

#include "ObsColumn.hh"
#include <QtGui/QComboBox>

// based on http://qt-project.org/wiki/Combo_Boxes_in_Item_Views

ObsDelegate::ObsDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}
 
ObsDelegate::~ObsDelegate()
{
}

QWidget* ObsDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QVariant valueType = index.data(ObsColumn::ValueTypeRole);
    if( not valueType.isValid() or valueType.toInt() != ObsColumn::TextCode )
        return QStyledItemDelegate::createEditor(parent, option, index);
 
    QComboBox *cb = new QComboBox(parent);
    QStringList codes = index.data(ObsColumn::TextCodesRole).toStringList();
    Q_FOREACH(const QString& c, codes)
        cb->addItem(c);
    return cb;
}
 
void ObsDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if(QComboBox *cb = qobject_cast<QComboBox*>(editor)) {
        QString currentText = index.data(Qt::EditRole).toString();
        int cbIndex = cb->findText(currentText);
        // if it is valid, adjust the combobox
        if(cbIndex >= 0)
            cb->setCurrentIndex(cbIndex);
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

void ObsDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if(QComboBox *cb = qobject_cast<QComboBox*>(editor))
        // save the current text of the combo box as the current value of the item
        model->setData(index, cb->currentText(), Qt::EditRole);
    else
        QStyledItemDelegate::setModelData(editor, model, index);
}
