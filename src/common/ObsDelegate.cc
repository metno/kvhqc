
#include "ObsDelegate.hh"

#include "common/ObsColumn.hh"
#include "util/ToolTipStringListModel.hh"

#include <QtGui/QComboBox>

// based on http://qt-project.org/wiki/Combo_Boxes_in_Item_Views

#define MILOGGER_CATEGORY "kvhqc.ObsDelegate"
#include "util/HqcLogging.hh"

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
    if (not valueType.isValid() or not (valueType.toInt() & ObsColumn::TextCode))
        return QStyledItemDelegate::createEditor(parent, option, index);

    QComboBox *cb = new QComboBox(parent);
    ToolTipStringListModel* ttl = new ToolTipStringListModel(cb);
    cb->setModel(ttl);
    return cb;
}

void ObsDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (QComboBox *cb = qobject_cast<QComboBox*>(editor)) {
        ToolTipStringListModel* ttl = static_cast<ToolTipStringListModel*>(cb->model());
        ttl->setStringList(index.data(ObsColumn::TextCodesRole).toStringList());
        ttl->setToolTipList(index.data(ObsColumn::TextCodeExplanationsRole).toStringList());

        const QVariant valueType = index.data(ObsColumn::ValueTypeRole);
        cb->setEditable(valueType.toInt() != ObsColumn::TextCode);

        QString currentText = index.data(Qt::EditRole).toString();
        int cbIndex = cb->findText(currentText);
        // if it is valid, adjust the combobox
        if(cbIndex >= 0)
            cb->setCurrentIndex(cbIndex);
        else
            cb->setEditText(currentText);
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

void ObsDelegate::updateEditorGeometry( QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
    QStyledItemDelegate::updateEditorGeometry(editor, option, index);
    QSize s = editor->size();
    editor->resize(s.expandedTo(QSize(100, 30)));
}
