
#ifndef HintWidget_hh
#define HintWidget_hh 1

#include <QtCore/QList>
#include <QtGui/QLabel>

QT_BEGIN_NAMESPACE
class QTextBrowser;
class QTimer;
QT_END_NAMESPACE

class HintWidget : public QLabel
{ Q_OBJECT;
public:
    HintWidget(QWidget* parent);
    ~HintWidget();

    void addHint(const QString& text);

private:
    void updateText();
    void updatePosition();

private Q_SLOTS:
    void hideHints();

private:
    QTextBrowser* mText;
    QTimer* mTimer;
    QList<QString> mHints;
};

#endif /* HintWidget_hh */
