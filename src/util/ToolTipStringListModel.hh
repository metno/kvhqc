
#ifndef ToolTipStringListModel_hh
#define ToolTipStringListModel_hh 1

#include <QStringListModel>

class ToolTipStringListModel : public QStringListModel {
public:
    ToolTipStringListModel(QObject* parent=0)
        : QStringListModel(parent) { }
    void setToolTipList(const QStringList& ttl);
    virtual QVariant data(const QModelIndex& index, int role) const;

private:
    QStringList mToolTips;
};

#endif // ToolTipStringListModel_hh
