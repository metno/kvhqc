
#ifndef OBSDELEGATE_HH
#define OBSDELEGATE_HH

#include <QtGui/QStyledItemDelegate>

class ObsDelegate : public QStyledItemDelegate
{   Q_OBJECT
public:
    ObsDelegate(QObject *parent = 0);
    ~ObsDelegate();

    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry( QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};

#endif // OBSDELEGATE_HH
