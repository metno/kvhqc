
#include "HintWidget.hh"

#include <QtCore/QTimer>
#include <QtGui/QAbstractTextDocumentLayout>
#include <QtGui/QApplication>
#include <QtGui/QPainter>
#include <QtGui/QTextDocument>

#include <boost/foreach.hpp>

#define NDEBUG
#include "debug.hh"

namespace {
const int margin = 6;
const float readingSpeed = 1000/35.0f; // milliseconds per character
}

HintWidget::HintWidget(QWidget* parent)
    : QWidget(parent, Qt::Tool|Qt::FramelessWindowHint)
    , mText(new QTextDocument(this))
    , mTimer(new QTimer(this))
{
    if (parent)
        setPalette(parent->palette());
    setVisible(false);

    mText->setUndoRedoEnabled(false);

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
    const int length = mText->toPlainText().size();
    const int timeout = (int) (length * readingSpeed);
    mTimer->start(timeout);
    qApp->processEvents();
}

void HintWidget::updateText()
{
    QString html = mHints.join("\n<hr />\n");
    mText->setHtml(html);
}

void HintWidget::updatePosition()
{
    QWidget* parent = static_cast<QWidget*>(this->parent());
    const QRect pr = parent->geometry();

    const int m = 2*margin, w = std::min(300, pr.width() - 2*m);
    mText->setTextWidth(w - m);
    mText->adjustSize();
    const QSize s = mText->size().toSize() + QSize(m, m);
    setGeometry(QRect(pr.bottomRight(), s).translated(-s.width()-m, -s.height()-m));
    raise();
}

void HintWidget::hideHints()
{
    mHints.clear();
    hide();
}

void HintWidget::paintEvent(QPaintEvent*)
{
    QRect r = rect();

    QPainter p(this);
    p.setPen(QPen(palette().toolTipText(), 0));
    p.setBrush(palette().toolTipBase());
    p.drawRect(r);

    p.setPen(QPen(Qt::blue, 4));
    p.setBrush(Qt::NoBrush);
    p.drawRect(r);

    r.adjust(margin, margin, -margin, -margin);
    p.translate(margin, margin);
    QRect rect = r;
    rect.translate(-margin, -margin);
    p.setClipRect(rect);

    QAbstractTextDocumentLayout::PaintContext context;
    mText->documentLayout()->draw(&p, context);
}
