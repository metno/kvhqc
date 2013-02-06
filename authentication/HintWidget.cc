
#include "HintWidget.hh"

#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QTextBrowser>

#include <boost/foreach.hpp>

//#define NDEBUG
#include "debug.hh"

namespace {
const int edge = 8;
const float readingSpeed = 1000/35.0f; // milliseconds per character
}

HintWidget::HintWidget(QWidget* parent)
    : QLabel(parent)
    , mText(new QTextBrowser(this))
    , mTimer(new QTimer(this))
{
    setStyleSheet(QString("QLabel { background-color: #ffd; border: 3px solid blue; border-radius: %1px; }").arg(edge));
    setVisible(false);

    mText->setOpenLinks(false);
    mText->viewport()->setAutoFillBackground(false);
    mText->setFrameShape(QTextEdit::NoFrame);
    mText->setFrameShadow(QTextEdit::Plain);

    mTimer->setSingleShot(true);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(hideHints()));
}

HintWidget::~HintWidget()
{
}

void HintWidget::addHint(const QString& hint)
{
    mHints << hint;
    updateText();
    updatePosition();
    show();
    const int size = mText->toPlainText().size();
    const int timeout = (int) (size * readingSpeed);
    mTimer->start(timeout);
    qApp->processEvents();
}

void HintWidget::updateText()
{
    QString html = "";
    BOOST_FOREACH(const QString& h, mHints) {
        html += h + "\n<hr />\n";
    }
    mText->setHtml(html);
}

void HintWidget::updatePosition()
{
    const QSize ps = static_cast<QWidget*>(parent())->size();
    const int pw = ps.width(), ph = ps.height();
    const int e = int(0.7*edge);
    const int w = std::min(300, pw - 2*e);
    const int h = std::min(200, ph - 2*e);
    setGeometry(pw - w - e, ph - h - e, w, h);
    mText->setGeometry(e, e, w - 2*e, h - 2*e);
    raise();
}

void HintWidget::hideHints()
{
    mHints.clear();
    hide();
}
