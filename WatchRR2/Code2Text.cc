
#include "Code2Text.hh"

#include "Helpers.hh"
#include <QtCore/QCoreApplication>
#include <boost/foreach.hpp>

Code2Text::Code2Text()
    : mMinValue(-100)
    , mMaxValue(5000)
    , mDecimals(1)
{
    addCode(kvalobs::NEW_ROW,  (QStringList() << qApp->translate("Code2Text", "new")),
            qApp->translate("Code2Text", "row not in database"));
    addCode(kvalobs::MISSING,  (QStringList() << qApp->translate("Code2Text", "mis")),
            qApp->translate("Code2Text", "value is missing"));
    addCode(kvalobs::REJECTED, (QStringList() << qApp->translate("Code2Text", "rej") << qApp->translate("Code2Text", "r")),
            qApp->translate("Code2Text", "value is rejected"));
}

Code2Text::~Code2Text()
{
}

QString Code2Text::asTip(float value)
{
    QMap<int,Code>::const_iterator it = mCodes.find(value);
    if (it == mCodes.end())
        return "";
    return QString("%2 (%1)").arg((int)value).arg(it.value().explain);
}

QString Code2Text::asText(float value)
{
    QMap<int,Code>::const_iterator it = mCodes.find(value);
    if (it == mCodes.end() or it.value().shortText.isEmpty())
        return QString::number(value, 'f', mDecimals);

    return it.value().shortText.front();
}

bool Code2Text::isCode(float value)
{
    QMap<int,Code>::const_iterator it = mCodes.find(value);
    return (it != mCodes.end());
}

float Code2Text::fromText(const QString& text)
{
    QMap<int, Code>::const_iterator it = mCodes.constBegin();
    ++it; ++it; // do not allow setting "mis" or "new"
    for (; it != mCodes.constEnd(); ++it) {
        const Code& c = it.value();
        if( c.shortText.contains(text) )
            return it.key();
    }

    const float num = Helpers::parseFloat(text, mDecimals);
    it = mCodes.find(num);
    if (it != mCodes.end())
        return num;
    if (num < mMinValue or num > mMaxValue)
        throw std::runtime_error("value out of range");
    return num;
}

void Code2Text::addCode(int value, const QStringList& shortText, const QString& explain)
{
    mCodes.insert(value, Code(shortText, explain));
}

void Code2Text::setRange(float mini, float maxi)
{
    mMinValue = mini;
    mMaxValue = maxi;
}

QStringList Code2Text::allCodes() const
{
  QStringList all;

  QMap<int, Code>::const_iterator it = mCodes.constBegin();
  ++it; ++it; // do not allow setting "mis" or "new"
  for (; it != mCodes.constEnd(); ++it) {
    if (not it.value().shortText.isEmpty())
      all << it.value().shortText.at(0);
  }
  return all;
}

QStringList Code2Text::allExplanations() const
{
  QStringList all;
  
  QMap<int, Code>::const_iterator it = mCodes.constBegin();
  ++it; ++it; // do not allow setting "mis" or new
  for (; it != mCodes.constEnd(); ++it)
    all << QString("%2 (%1)").arg(it.key()).arg(it.value().explain);
  return all;
}
