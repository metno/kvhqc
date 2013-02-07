
#ifndef HintWidget_hh
#define HintWidget_hh 1

#include <QtCore/QStringList>
#include <QtGui/QLabel>

QT_BEGIN_NAMESPACE
class QTextDocument;
class QTimer;
QT_END_NAMESPACE

class HintWidget : public QWidget
{ Q_OBJECT;
public:
    HintWidget(QWidget* parent);
    ~HintWidget();

    void addHint(const QString& text);

public Q_SLOTS:
    void updatePosition();

protected:
    void paintEvent(QPaintEvent*);

private:
    void updateText();

private Q_SLOTS:
    void hideHints();

private:
    QTextDocument* mText;
    QTimer* mTimer;
    QStringList mHints;
};

#endif /* HintWidget_hh */
