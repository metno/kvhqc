
#ifndef ITEMCHECKBOX_HH
#define ITEMCHECKBOX_HH 1

#include <QCheckBox>

class ItemCheckBox : public QCheckBox
{ Q_OBJECT
public:
    ItemCheckBox(QString label, QString item, QWidget* parent=0)
        : QCheckBox(label, parent), mItem(item) { }
    QString getItem() const { return mItem; }
private Q_SLOTS:
    void clicked();
Q_SIGNALS:
    void clicked(QString item);
private:
    QString mItem;
};

#endif // ITEMCHECKBOX_HH
