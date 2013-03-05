
#include "ObsDelegate.hh"

#include "ObsColumn.hh"
#include <QtGui/QComboBox>
#include <QtGui/QStringListModel>

// based on http://qt-project.org/wiki/Combo_Boxes_in_Item_Views

#define NDEBUG
#include "debug.hh"

class TooltipListStringListModel : public QStringListModel {
public:
    TooltipListStringListModel(QObject* parent=0)
        : QStringListModel(parent) { }
    void setToolTipList(const QStringList& ttl);
    virtual QVariant data(const QModelIndex& index, int role) const;

private:
    QStringList mToolTips;
};

void TooltipListStringListModel::setToolTipList(const QStringList& ttl)
{
    mToolTips = ttl;
    if (rowCount() != mToolTips.size()) {
        LOG4HQC_ERROR("TooltipListStringListModel", "have " << rowCount() << " items but "
                      << mToolTips.size() << " tooltips: [" << mToolTips.join(",") << "]");
    }
}

QVariant TooltipListStringListModel::data(const QModelIndex& index, int role) const
{
    if (role != Qt::ToolTipRole)
        return QStringListModel::data(index, role);
    if (index.row() >= mToolTips.size()) {
        LOG4HQC_ERROR("TooltipListStringListModel", "tooltip for item " << index.row() << " requested, but only "
                      << mToolTips.size() << " tooltips: [" << mToolTips.join(",") << "]");
        return QVariant();
    }
    return mToolTips.at(index.row());
}

// ########################################################################

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
    TooltipListStringListModel* ttl = new TooltipListStringListModel(cb);
    cb->setModel(ttl);
    return cb;
}

void ObsDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (QComboBox *cb = qobject_cast<QComboBox*>(editor)) {
        TooltipListStringListModel* ttl = static_cast<TooltipListStringListModel*>(cb->model());
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
            cb->setCurrentText(currentText);
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
